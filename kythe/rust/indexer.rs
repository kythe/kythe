//! Parses Rust code from kythe CompilationUnits and emits kythe graph
//! fragments.
//!
//! Input and output are abstracted: see kzip_indexer and stubby_indexer for
//! concrete entry points.
//!
//! The indexer uses rust-analyzer's parser and semantic analysis, so generally
//! is not completely precise.

// Deps:
//   :entry_writer
//   :file_range
//   :hir_to_syntax
//   :indexing_ctx
//   :parser 
//   :vnames
//   crates.io:log
//   kythe:analysis_rust_proto
//   kythe:common_rust_proto
//   kythe:storage_rust_proto
//   protobuf:protobuf
//   crates.io:anyhow
//   rust_analyzer

use analysis_rust_proto::{analysis_result, AnalysisResult, CompilationUnitView};
use common_rust_proto::{marked_source, MarkedSource};
use entry_writer::{EdgeKind, EntryWriter, NodeKind, NodeSubkind};
use file_range::FileRange;
use hir_to_syntax::ToSyntaxNode;
use indexing_ctx::IndexingContext;
use protobuf::proto;
use rust_analyzer::{
    edition::Edition,
    hir::{
        sym, Adt, AsAssocItem, AssocItem, AssocItemContainer, Const, DisplayTarget, Field,
        Function, GenericDef, GenericParam, HasAttrs, HirDisplay, Impl, InFile, Local, Macro,
        MacroKind, Module, ModuleSource, Semantics, Static, Symbol, Trait, TraitAlias, TypeAlias,
        Variant,
    },
    hir_def::attr::AttrQuery,
    ide::{AssistResolveStrategy, DiagnosticsConfig, Severity},
    ide_db::{
        defs::{Definition, IdentClass, NameClass, NameRefClass, OperatorClass},
        documentation::HasDocs,
    },
    ide_diagnostics::full_diagnostics,
    syntax::{ast, AstNode, SyntaxNode},
    vfs::FileId,
};
use std::path::Path;
use storage_rust_proto::EntryView;
use vnames::VNameFactory;

pub use parser::FileDigest;
pub use rust_analyzer::paths::{AbsPath, AbsPathBuf};

/// Analyze a compilation unit and produce kythe graph fragments describing it.
pub fn index(
    cu: CompilationUnitView,
    // Reads file content for a given digest.
    read: &mut impl FnMut(&FileDigest) -> anyhow::Result<Vec<u8>>,
    // The path to the standard library.
    //
    // rust-analyzer's sysroot support is not VFS-clean.
    // This will be both through the real FS (sysroot detection) and VFS
    // (parsing). Therefore it must be an absolute path to a real physical
    // directory.
    sysroot_src: Option<&AbsPath>,
    // The path to the proc macro dynamic libraries.
    // This is a proc_macro_bundle, see proc_macros/BUILD.
    proc_macros: Option<&Path>,
    // This function is called with each node/edge in the produced index.
    write: &mut impl FnMut(EntryView),
    // Use vnames that are less unique, but easier to read/assert.
    abbreviate: bool,
) -> AnalysisResult {
    let parse = parser::parse(cu, sysroot_src, proc_macros, read);
    let Ok(parser::Parse { sources, krate, special, host, vnames }) = parse else {
        return proto!(AnalysisResult {
            status: analysis_result::Status::InvalidRequest,
            summary: parse.unwrap_err().to_string()
        });
    };

    let db = host.raw_database();
    let sema = Semantics::new(db);

    let vnames = VNameFactory::new(&sema, cu.v_name().to_owned(), vnames, special, abbreviate);

    let mut ctx = IndexingContext::new(krate, &sema, &vnames);
    ctx.enqueue(Definition::from(krate.root_module()));

    // Index source files themselves + traverse the contained syntax in search of name refs.
    for (file_id, source) in &sources {
        let vname = vnames.file(*file_id);
        let entry = EntryWriter::new(vname, write, &vnames, &sema);
        index_file(*file_id, source, &mut ctx, entry);
    }

    // Process the queue of things to index.
    while let Some(def) = ctx.pop_queue() {
        let result = vnames.definition(def).and_then(|vname| {
            let entry = EntryWriter::new(vname, write, &vnames, &sema);
            index_definition(def, &mut ctx, entry)
        });
        if let Err(err) = result {
            log::warn!("failed to index: {err}");
        }
    }

    AnalysisResult::default()
}

fn index_file(file_id: FileId, source: &[u8], ctx: &mut IndexingContext, mut entry: EntryWriter) {
    entry.kind(NodeKind::File, None);
    entry.text(source);

    index_error_diagnostics(file_id, ctx, &mut entry);
    index_syntax(ctx.sema.parse_guess_edition(file_id).syntax(), ctx, &mut entry);
}

// Add 'ref' edges for any AST name refs, and enqueue any definitions found.
//
// Traversing the syntax is the most reliable way to find all references to definitions, so we
// create those anchors and `ref` edges here.
//
// We do not emit anchors and edges (`defines/binding`) for definitions - these are easier to
// find and classify when visiting the HIR nodes. However we do want to find and *enqueue* the HIR
// node whenever we see such a definition, as our top-down HIR traversal won't find everything
// (for example certain private symbols).
#[rustfmt::skip::macros(match_ast)] // macro needs commas, rustfmt strips them.
fn index_syntax(root: &SyntaxNode, ctx: &mut IndexingContext, entry: &mut EntryWriter) {
    for node in root.descendants() {
        // Descend into macros to find more references.
        if let Some(ast) = ast::MacroCall::cast(node.clone()) {
            if let Some(expanded) = ctx.sema.expand_macro_call(&ast) {
                index_syntax(&expanded, ctx, entry);
            }
        }
        let Some(class) = IdentClass::classify_node(ctx.sema, &node) else { continue };
        // All referenced nodes should be indexed.
        let all_defs: Vec<_> = class.definitions().into_iter().map(|(def, _subst)| def).collect();
        ctx.enqueue_all(all_defs.iter().cloned());
        // We have to classify_node again as definitions() consumes IdentClass :-(
        let refs = referenced_defs(IdentClass::classify_node(ctx.sema, &node).unwrap());
        // Emit ref edges to each referenced definition.
        for r in refs {
            entry.target = match ctx.vnames.definition(r) {
                Ok(vname) => vname,
                Err(err) => {
                    log::warn!("failed to index from AST: {err}");
                    continue;
                }
            };
            entry.anchor(EdgeKind::Ref, InFile::new(ctx.sema.hir_file_for(&node), node.clone()));
        }
    }
}

// If this references a definition, which one(s)?
fn referenced_defs(reference: IdentClass) -> Vec<Definition> {
    fn all_defs(reference: IdentClass) -> Vec<Definition> {
        reference.definitions().into_iter().map(|(def, _subst)| def).collect()
    }
    match reference {
        IdentClass::NameRefClass(NameRefClass::ExternCrateShorthand { krate, .. }) => {
            vec![Definition::Crate(krate)]
        }
        IdentClass::NameRefClass(_) => all_defs(reference),
        // Some operators are uninteresting to treat as refs - acting more as part of the language.
        IdentClass::Operator(OperatorClass::Range(_))
        | IdentClass::Operator(OperatorClass::Try(_)) => Vec::new(),
        IdentClass::Operator(_) => all_defs(reference),
        IdentClass::NameClass(NameClass::ConstReference(_)) => all_defs(reference),
        IdentClass::NameClass(NameClass::PatFieldShorthand { field_ref, .. }) => {
            vec![Definition::Field(field_ref)]
        }
        IdentClass::NameClass(_) => Vec::new(),
    }
}

fn index_error_diagnostics(file_id: FileId, ctx: &mut IndexingContext, entry: &mut EntryWriter) {
    // Enqueue any error diagnostics.
    let diags_config = DiagnosticsConfig {
        style_lints: false,
        term_search_fuel: 0,
        ..DiagnosticsConfig::test_sample()
    };
    let diags = full_diagnostics(ctx.db, &diags_config, &AssistResolveStrategy::None, file_id);
    let error_diags = diags.into_iter().filter(|diag| diag.severity == Severity::Error);

    for diag in error_diags {
        entry.target = ctx.vnames.diagnostic(file_id, &diag);
        entry.kind(NodeKind::Diagnostic, None);
        entry.message(diag.message.as_bytes());
        entry.range_anchor(EdgeKind::Tagged, diag.range.into());
    }
}

fn index_definition(
    def: Definition,
    ctx: &mut IndexingContext,
    entry: EntryWriter,
) -> anyhow::Result<()> {
    match def {
        Definition::Macro(def) => index_macro(def, ctx, entry),
        Definition::Field(def) => index_field(def, ctx, entry),
        Definition::Module(def) => index_module(def, ctx, entry),
        Definition::Function(def) => index_function(def, ctx, entry),
        Definition::Adt(def) => index_adt(def, ctx, entry),
        Definition::Variant(def) => index_variant(def, ctx, entry),
        Definition::Const(def) => index_const(def, ctx, entry),
        Definition::Static(def) => index_static(def, ctx, entry),
        Definition::Trait(def) => index_trait(def, ctx, entry),
        Definition::TraitAlias(def) => index_trait_alias(def, ctx, entry),
        Definition::TypeAlias(def) => index_type_alias(def, ctx, entry),
        Definition::SelfType(def) => index_impl(def, ctx, entry),
        Definition::GenericParam(def) => index_generic_param(def, ctx, entry),
        Definition::Local(def) => index_local(def, ctx, entry),
        _ => anyhow::bail!("could not index unsupported definition kind: {def:?}"),
    }
}

fn index_macro(
    def: Macro,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Macro, None);
    match def.kind(ctx.db) {
        // macro-by-example/macro_rules!
        MacroKind::Declarative => {
            if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
                entry.anchor(EdgeKind::Defines, syntax.clone());
                entry.name_anchor(EdgeKind::DefinesBinding, syntax);
            }
        }
        _ => {}
    }
    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    Ok(())
}

fn index_field(
    def: Field,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Variable, Some(NodeSubkind::Field));
    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }
    entry.edge_to(EdgeKind::ChildOf, Definition::from(def.parent_def(ctx.db)));
    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);
    Ok(())
}

fn index_module(
    def: Module,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Package, None);

    // Output entries specific to particular kinds of modules (source file vs item).
    let hir_file_id = def.definition_source_file_id(ctx.db);
    match def.definition_source(ctx.db).value {
        ModuleSource::Module(ast_module) => {
            let syntax = InFile::new(hir_file_id, ast_module.syntax().clone());
            entry.anchor(EdgeKind::Defines, syntax.clone());
            entry.name_anchor(EdgeKind::DefinesBinding, syntax);
        }
        ModuleSource::SourceFile(_) => {
            let editioned_file_id = hir_file_id.original_file(ctx.db);
            let file_id = editioned_file_id.file_id(ctx.db);

            // Following the advice in https://kythe.io/docs/schema/#package, a zero range
            // defines/implicit modules that are defined through source files.
            let zero_range = FileRange { file_id, start: 0, end: 0 };
            entry.range_anchor(EdgeKind::DefinesImplicit, zero_range);

            // Create a ref edge from the `mod foo;` declaration that exists in some other file
            // that points to this module.
            if let Some(decl_ast) = def.declaration_source(ctx.db) {
                let decl_syntax = decl_ast.map(|ast| ast.syntax().clone());
                entry.name_anchor(EdgeKind::Ref, decl_syntax);
            }
        }
        ModuleSource::BlockExpr(_) => unreachable!(),
    }

    if let Some(parent) = def.parent(ctx.db) {
        entry.edge_to(EdgeKind::ChildOf, Definition::from(parent));
    }

    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);

    // Index all the impl blocks in the module. This includes both inherent and trait impls
    // (except those defined inside of other items).
    let defs = def.impl_defs(ctx.db);
    ctx.enqueue_all(defs.into_iter().map(Definition::from));

    // Index all visible definitions in the module. This does not include nested items.
    let defs = def.declarations(ctx.db);
    ctx.enqueue_all(defs.into_iter().map(Definition::from));

    Ok(())
}

fn index_function(
    def: Function,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Function, None);

    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }

    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_generic_params(def, ctx, &mut entry);
    handle_child_of_impl_or_trait(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);

    // `assoc_fn_params` is perhaps a misleading name but all it does is return all function
    // parameters including the `self` parameter if there is one.
    for param in def.assoc_fn_params(ctx.db) {
        // TODO(b/405350061): Figure out how best to express in the graph pattern matching in
        // function parameter position. For now, we just handle params that aren't doing any
        // pattern matching.
        if let Some(local) = param.as_local(ctx.db) {
            let local = Definition::from(local);
            entry.edge_to(EdgeKind::Param(param.index()), local);
            ctx.enqueue(local);
        }
    }

    Ok(())
}

fn index_adt(def: Adt, ctx: &mut IndexingContext, mut entry: EntryWriter) -> anyhow::Result<()> {
    let (kind, subkind) = match def {
        Adt::Struct(_) => (NodeKind::Record, NodeSubkind::Struct),
        Adt::Union(_) => (NodeKind::Sum, NodeSubkind::Union),
        Adt::Enum(_) => (NodeKind::Sum, NodeSubkind::Enum),
    };
    entry.kind(kind, Some(subkind));

    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }

    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_generic_params(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);

    match def {
        Adt::Struct(hir_struct) => {
            ctx.enqueue_all(hir_struct.fields(ctx.db).into_iter().map(Definition::from))
        }
        Adt::Union(hir_union) => {
            ctx.enqueue_all(hir_union.fields(ctx.db).into_iter().map(Definition::from))
        }
        Adt::Enum(hir_enum) => {
            ctx.enqueue_all(hir_enum.variants(ctx.db).into_iter().map(Definition::from))
        }
    }

    Ok(())
}

fn index_variant(
    def: Variant,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    // All variants, including those without payloads, are modelled as records for consistency.
    entry.kind(NodeKind::Record, None);

    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }

    let parent = Definition::from(Adt::from(def.parent_enum(ctx.db)));
    entry.edge_to(EdgeKind::Extends, parent);

    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);

    ctx.enqueue_all(def.fields(ctx.db).into_iter().map(Definition::from));

    Ok(())
}

fn index_const(
    def: Const,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Constant, None);
    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }
    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_child_of_impl_or_trait(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);
    Ok(())
}

fn index_static(
    def: Static,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Variable, None);
    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }
    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);
    Ok(())
}

fn index_trait(
    def: Trait,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Interface, None);

    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }

    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_generic_params(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);

    ctx.enqueue_all(def.items(ctx.db).into_iter().map(Definition::from));

    Ok(())
}

fn index_trait_alias(
    def: TraitAlias,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::TypeAlias, None);
    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }
    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_generic_params(def, ctx, &mut entry);
    // TODO(b/407042236): The output of `HirDisplay::display` for trait aliases is strange. If not
    // fixed upstream, we should manually construct a more reasonable display string before trait
    // aliases become a stable language feature.
    handle_code_hir_display(def, ctx, &mut entry);
    // TODO(b/405349357): Add aliases edge from alias to trait node.
    Ok(())
}

fn index_type_alias(
    def: TypeAlias,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::TypeAlias, None);
    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());
        entry.name_anchor(EdgeKind::DefinesBinding, syntax);
    }
    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_generic_params(def, ctx, &mut entry);
    handle_child_of_impl_or_trait(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);
    // TODO(b/405349357): Add aliases edge from alias to type node.
    Ok(())
}

fn index_impl(def: Impl, ctx: &mut IndexingContext, mut entry: EntryWriter) -> anyhow::Result<()> {
    entry.kind(NodeKind::Record, None);

    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        // Impls are defined by their full AST node. They have no defines/binding edge.
        entry.anchor(EdgeKind::Defines, syntax);
    }

    handle_attrs(def, ctx, &mut entry);
    handle_doc_comments(def, ctx, &mut entry);
    handle_generic_params(def, ctx, &mut entry);

    let impled_on = def.self_ty(ctx.db);
    // In simple cases of `impl Foo` where `Foo` is some ADT, it is trivial to add an 'extends'
    // edge. We do not attempt to add edges for more complex types (constituent parts of more
    // complex types will still be 'lookup-able' thanks to indexing of name refs).
    if let Some(adt) = impled_on.as_adt() {
        entry.edge_to(EdgeKind::Extends, adt.into());
    }

    if let Some(impled_trait) = def.trait_(ctx.db) {
        entry.edge_to(EdgeKind::Extends, impled_trait.into());
    }

    ctx.enqueue_all(def.items(ctx.db).into_iter().map(Definition::from));

    Ok(())
}

fn index_generic_param(
    def: GenericParam,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::TypeVariable, None);

    // We return early on implicit type parameters (there are no useful anchors to add). A type
    // parameter is implicit if it is `Self` or created implicitly through `impl Trait`.
    match def {
        GenericParam::TypeParam(type_param) if type_param.is_implicit(ctx.db) => return Ok(()),
        _ => {}
    }

    if let Some(syntax) = def.to_in_file_syntax_node(ctx.sema) {
        entry.anchor(EdgeKind::Defines, syntax.clone());

        // `ast::LifetimeParam` does not impl `ast::HasName` so we handle it as a special case.
        if let Some(param) = ast::LifetimeParam::cast(syntax.value.clone()) {
            if let Some(lt) = param.lifetime() {
                let syntax = syntax.map(|_| lt.syntax().clone());
                entry.anchor(EdgeKind::DefinesBinding, syntax);
            }
        } else {
            entry.name_anchor(EdgeKind::DefinesBinding, syntax);
        }
    }

    handle_attrs(def, ctx, &mut entry);
    handle_code_hir_display(def, ctx, &mut entry);

    Ok(())
}

fn index_local(
    def: Local,
    ctx: &mut IndexingContext,
    mut entry: EntryWriter,
) -> anyhow::Result<()> {
    entry.kind(NodeKind::Variable, None);

    for source in def.sources(ctx.db) {
        if let Some(syntax) = source.to_in_file_syntax_node(ctx.sema) {
            entry.name_anchor(EdgeKind::DefinesBinding, syntax);
        }
    }

    // `hir::Local` does not implement `HirDisplay` so we construct the doc text ourselves.
    let target = DisplayTarget::from_crate(ctx.db, ctx.krate.into());
    let mut text = match def.as_self_param(ctx.db) {
        // We use `hir::SelfParam`'s impl of `HirDisplay` (instead of just converting the name to a
        // string directly) as it will include the preceeding `&`/`&mut`/etc. if present.
        Some(param) => param.display(ctx.db, target).to_string(),
        None => def.name(ctx.db).display(ctx.db, Edition::CURRENT).to_string(),
    };
    text.push_str(&format!(": {}", def.ty(ctx.db).display(ctx.db, target)));
    handle_code_text(&text, &mut entry);

    // TODO(b/405352545): Typed edges from local to its type.

    Ok(())
}

fn handle_attrs(def: impl HasAttrs, ctx: &mut IndexingContext, entry: &mut EntryWriter) {
    let attrs = def.attrs(ctx.db);

    let deprecated = attrs.by_key(sym::deprecated);
    if deprecated.clone().exists() {
        handle_deprecated_attr(deprecated, entry);
    }
}

fn handle_deprecated_attr(attr: AttrQuery<'_>, entry: &mut EntryWriter) {
    // Handle `#[deprecated = "..."]`.
    if let Some(msg) = attr.clone().string_value() {
        entry.deprecated(msg.as_str());
        return;
    }

    // Handle `#[deprecated]` and `#[deprecated(since = "...", note = "...")]`.

    let mut msg = String::new();

    let sym_since = Symbol::intern("since");
    let since = attr.clone().find_string_value_in_tt(sym_since);
    if let Some(since) = since {
        msg.push_str("since ");
        msg.push_str(since);
    }

    let sym_note = Symbol::intern("note");
    if let Some(note) = attr.find_string_value_in_tt(sym_note) {
        if since.is_some() {
            msg.push_str(": ");
        }
        msg.push_str(note);
    }

    entry.deprecated(&msg);
}

fn handle_doc_comments(
    def: impl HasDocs + Into<Definition> + Copy,
    ctx: &mut IndexingContext,
    entry: &mut EntryWriter,
) {
    let Some(doc) = def.docs(ctx.db) else { return };
    // TODO(b/407042236): Handle links in doc comments.
    let text = String::from(doc).replace(r"\", r"\\").replace("[", r"\[").replace("]", r"\]");

    // TODO(b/407760074): Mutating an existing `EntryWriter` and then explicitly changing it back
    // to its previous state seems error prone.
    let previous_target = entry.target.clone();
    let Ok(doc_target) = ctx.vnames.doc(def.into()) else { return };
    entry.target = doc_target;

    entry.kind(NodeKind::Doc, None);
    entry.text(text.as_bytes());
    entry.edge_to(EdgeKind::Documents, def.into());

    entry.target = previous_target;
}

fn handle_generic_params(
    def: impl Into<GenericDef>,
    ctx: &mut IndexingContext,
    entry: &mut EntryWriter,
) {
    for (i, param) in def.into().params(ctx.db).into_iter().enumerate() {
        let param = Definition::from(param);
        entry.edge_to(EdgeKind::TypeParam(i), param);
        ctx.enqueue(param);
    }
}

fn handle_child_of_impl_or_trait(
    item: impl AsAssocItem,
    ctx: &mut IndexingContext,
    entry: &mut EntryWriter,
) {
    let Some(item) = item.as_assoc_item(ctx.db) else { return };

    let container: Definition = match item.container(ctx.db) {
        AssocItemContainer::Impl(container_impl) => {
            item_in_impl_overrides_item_in_trait(item, container_impl, ctx, entry);
            container_impl.into()
        }
        AssocItemContainer::Trait(container_trait) => container_trait.into(),
    };

    entry.edge_to(EdgeKind::ChildOf, container);
}

fn item_in_impl_overrides_item_in_trait(
    item: AssocItem,
    container_impl: Impl,
    ctx: &mut IndexingContext,
    entry: &mut EntryWriter,
) {
    if let Some(impled_trait) = container_impl.trait_(ctx.db) {
        let item_in_trait = impled_trait
            .items(ctx.db)
            .into_iter()
            .find(|candidate| candidate.name(ctx.db) == item.name(ctx.db));
        if let Some(item_in_trait) = item_in_trait {
            entry.edge_to(EdgeKind::Overrides, item_in_trait.into());
        }
    }
}

fn handle_code_hir_display(
    def: impl HirDisplay,
    ctx: &mut IndexingContext,
    entry: &mut EntryWriter,
) {
    // TODO(b/407042236): Encode structure in MarkedSource instead of an opaque string. Probably
    // doable using `HirDisplay::write_to` instead of `display`.
    let target = DisplayTarget::from_crate(ctx.db, ctx.krate.into());
    let text = def.display(ctx.db, target).to_string();
    handle_code_text(&text, entry);
}

fn handle_code_text(text: &str, entry: &mut EntryWriter) {
    let marked_source = proto!(MarkedSource { kind: marked_source::Kind::Type, pre_text: text });
    entry.code(marked_source.as_view());
}
