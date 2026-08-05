//! This module produces identifiers for the Kythe graph nodes we emit.
//!
//! The goal is that for each Rust entity (e.g. a function) that the graph will
//! describe, there is one stable identifier that the crate defining it and
//! all crates referencing it will agree on.
//! This ensures that when each crate is indexed and the results are merged,
//! we produce a coherent graph with references linked up to definitions.
//!
//! Nodes identifiers are vnames: (corpus, path, language, signature) tuples.
//! The corpus, path, and language have specified meanings, but we have free
//! choice of how to structure the signature.
//!
//! For named entities, we try to make the signature resemble paths:
//! `[crate mycrate.rs]/foo/Bar`

// Deps:
//   :file_range
//   :hir_to_syntax
//   kythe:storage_rust_proto
//   crates.io:anyhow
//   rust_analyzer

use std::collections::HashMap;

use anyhow::Context;
use file_range::FileRange;
use hir_to_syntax::ToSyntaxNode;
use rust_analyzer::{
    base_db::{CrateOrigin, LangCrateOrigin},
    hir::{Adt, Crate, GenericParam, HasContainer, InFile, ItemContainer, Module, Semantics},
    hir_def::ModuleId,
    ide::RootDatabase,
    ide_db::defs::Definition,
    ide_diagnostics::Diagnostic,
    syntax::{AstNode, SyntaxNode},
    vfs::FileId,
};
use storage_rust_proto::VName;

/// The helper object used to generate VNames.
/// (Maybe in future it will cache them).
pub struct VNameFactory<'a> {
    sema: &'a Semantics<'a, RootDatabase>,
    base: VName,
    files: HashMap<FileId, VName>,
    special_crates: HashMap<Crate, LangCrateOrigin>,
    // Use simplified vnames that are less unique, but easier to read/assert.
    abbreviate: bool,
}

impl VNameFactory<'_> {
    pub fn new<'a>(
        sema: &'a Semantics<'a, RootDatabase>,
        mut base: VName,
        files: HashMap<FileId, VName>,
        special_crates: HashMap<Crate, LangCrateOrigin>,
        abbreviate: bool,
    ) -> VNameFactory<'a> {
        base.set_path("");
        VNameFactory { sema, base, files, special_crates, abbreviate }
    }

    /// A vname for the specified file. These are provided by kythe, in the CU.
    /// Note that file vnames have no language!
    pub fn file(&self, file_id: FileId) -> VName {
        self.files.get(&file_id).expect("unknown file").clone()
    }

    /// A vname for a textual range within a file.
    ///
    /// Anchors are matched against files based on (corpus, path).
    /// Unlike file vnames, these include the language.
    pub fn anchor(&self, range: FileRange) -> VName {
        self.local(range.file_id, format!("[{}:{}]", range.start, range.end))
    }

    /// A vname for an error diagnostic message. This is just a file-local vname with a signature in
    /// a specific format.
    pub fn diagnostic(&self, file_id: FileId, diag: &Diagnostic) -> VName {
        self.local(
            file_id,
            format!(
                "diag:{}:{}:{}",
                diag.code.as_str(),
                usize::from(diag.range.range.start()),
                usize::from(diag.range.range.end())
            ),
        )
    }

    /// The vname for a Rust-defined entity with a signature expressed primarily in terms of the
    /// semantic representation of that entity (with source code span information encoded where
    /// necessary to ensure uniqueness).
    ///
    /// We do not set `path`, even for file-local entities. The signature is a path rooted at a
    /// crate and separated by '/'. e.g., `[crate mycrate.rs]/tests/Fixture/name`.
    pub fn definition(&self, def: Definition) -> anyhow::Result<VName> {
        let mut vname = self.base.clone();
        vname.set_signature(self.definition_path(def)?);
        Ok(vname)
    }

    /// Documentation nodes for definitions also need vnames. For this, we just append text to the
    /// signature of the definition itself.
    pub fn doc(&self, def: Definition) -> anyhow::Result<VName> {
        let mut vname = self.definition(def)?;
        vname.set_signature(format!("{}/doc", vname.signature()));
        Ok(vname)
    }

    /// A vname associated with a file, with an arbitrary signature within it.
    ///
    /// This is used for things like diagnostics that are inherently textual. In other cases we
    /// leave the path empty and use semantic identifiers in the signature instead.
    fn local(&self, file_id: FileId, signature: String) -> VName {
        let mut vname = self.file(file_id);
        vname.set_language("rust");
        vname.set_signature(signature);
        vname
    }

    /// Construct a path to a definition originating in some crate. The constructed path is used as a
    /// vname signature to uniquely refer to that definition.
    fn definition_path(&self, def: Definition) -> anyhow::Result<String> {
        // Base case: crate (or equivalently, its root module).
        match def {
            Definition::Crate(krate) => return self.crate_signature(krate),
            Definition::Module(m) if m.is_crate_root() => return self.crate_signature(m.krate()),
            _ => {}
        }

        let segment = definition_segment(def, self.sema).context("unsupported definition")?;
        let mut path = self.definition_path(segment.parent)?; // recursive call
        path.push('/');
        match segment.namespace {
            Some(Namespace::Type) => path.push('~'),
            Some(Namespace::Value) => path.push('='),
            Some(Namespace::Macro) => path.push('!'),
            Some(Namespace::Lifetime) => path.push('\''),
            Some(Namespace::Label) => path.push('@'),
            Some(Namespace::Field) => path.push('.'),
            None => {}
        }
        path.push_str(&segment.identifier);
        if let Some(position) = segment.position {
            path.push_str(&format!("[{}:{}]", position.start, position.end));
        }
        Ok(path)
    }

    /// Determine whether a definition originated in a standard library crate or in a bazel crate.
    fn crate_signature(&self, krate: Crate) -> anyhow::Result<String> {
        let special_origin = match krate.origin(self.sema.db) {
            CrateOrigin::Lang(origin) => Some(origin),
            _ => self.special_crates.get(&krate).cloned(),
        };
        match special_origin {
            // Definitions originating in the standard library will have signatures prefixed with
            // the crate name (e.g., "core/...").
            Some(origin) => Ok(origin.to_string()),
            // Definitions originating in user code will be prefixed with the file path of the root
            // module of the crate in which they are defined (e.g., "[crate foo/bar.rs]/...").
            // This is slightly confusing: it will be part of the signature of the vname rather
            // than the path, and will appear in the signature of definitions in other files.
            None => {
                let file =
                    self.files.get(&krate.root_file(self.sema.db)).expect("missing file").path();
                Ok(if self.abbreviate {
                    let basename = file.as_bytes().rsplit(|c| *c == b'/').next().unwrap();
                    format!("[{}]", String::from_utf8_lossy(basename))
                } else {
                    format!("[crate {}]", file)
                })
            }
        }
    }

    // TODO(b/405352545): we'll need vnames for entities without definitions, like
    // types. These do not fit into the above scheme: the type `(i32, String)`
    // has no path and is not rooted at any particular crate.
}

/// A single segment in a larger path to a Rust definition.
struct Segment {
    /// The name/identifier of a definition (or a fixed string for unnamed entities like impls).
    identifier: String,
    /// In Rust, names can belong to one of multiple distinct namespaces. To avoid conflicting
    /// vnames, path segments must record the namespace in which their name is found.
    namespace: Option<Namespace>,
    /// The parent of the definition that produced this segment. This can later be used to construct
    /// the preceeding segment in a larger path.
    parent: Definition,
    /// In cases where the parent/child relationship cannot be uniquely expressed semantically due
    /// to the limited information that HIR records (it may not record a parent/child relationship
    /// between certain nested items for example) we also record a source code position.
    position: Option<FileRange>,
}

// Returns the definition corresponding to a (pseudo-)Module, if any.
//
// Rust-analyzer creates pseudo-modules to represent scopes that might contain functions, types etc.
// Given the following code:
//
//     fn foo() {
//       struct S;     // want vname: [crate]/=foo/~S
//       if true {
//         struct T;   // want vname: [crate]root/=foo/~T[...]     ("T" might not be unique in foo)
//       }
//     }
//
// We have the following enclosing-definition tree:
//
//           root
//          /    \
//       mod#0   foo()
//        /   \
//      mod#1  S
//        |
//        T
//
// To generate the correct vnames, we need to determine:
//    - root is a real module       ==> resolves to itself
//    - mod#0 is a function body    ==> resolves to the function
//    - mod#1 is an anonymous block ==> does not resolve to a def
fn resolve_module(module: Module, _sema: &Semantics<RootDatabase>) -> Option<Definition> {
    let id: ModuleId = module.into();
    if !id.is_block_module() {
        return Some(Definition::Module(module));
    }
    // TODO(b/406714387): the below produces better vnames, but sometimes crashes due to `to_node()`
    // re-parsing and returning a non-canonical syntax node that `to_def` rejects.
    /*
        let block = id.containing_block()?.lookup(sema.db).ast_id.to_node(sema.db);
        let function = block.syntax().parent().and_then(ast::Fn::cast)?;
        sema.to_def(&function).map(Definition::Function)
    */
    None
}

// Returns the definition we consider the parent for vname purposes.
//
// The child may be in some anonymous local scope within the parent.
// For instance, a struct may be defined inside an `if { ... }` in a function.
// This means the struct's name need not be unique within the function.
fn parent_def(def: Definition, sema: &Semantics<RootDatabase>) -> Option<(Definition, bool)> {
    let db = sema.db;
    let from_container = |container: ItemContainer| -> Option<Definition> {
        match container {
            ItemContainer::Trait(it) => Some(it.into()),
            ItemContainer::Impl(it) => Some(it.into()),
            ItemContainer::Module(it) => Some(it.into()),
            ItemContainer::ExternBlock(_) => def.module(db).map(Into::into),
            ItemContainer::Crate(_) => Some(Definition::Crate(def.krate(db)?)),
        }
    };
    let mut enclosing = match def {
        Definition::Crate(_) => return None,
        // Items that may apear in various kinds of container, such as impls.
        Definition::Function(it) => from_container(it.container(db))?,
        Definition::Const(it) => from_container(it.container(db))?,
        Definition::Static(it) => from_container(it.container(db))?,
        Definition::Trait(it) => from_container(it.container(db))?,
        Definition::TraitAlias(it) => from_container(it.container(db))?,
        Definition::TypeAlias(it) => from_container(it.container(db))?,
        Definition::ExternCrateDecl(it) => from_container(it.container(db))?,
        // Items that may only appear in modules (which may be block-scope pseudomodules).
        Definition::Adt(it) => it.module(db).into(),
        Definition::SelfType(it) => it.module(db).into(),
        Definition::Macro(it) => it.module(db).into(),
        Definition::Module(it) => Definition::Module(it.parent(db)?),
        // Items that appear inside bodies (always block-scope pseudomodules?).
        Definition::Local(it) => it.parent(db).try_into().ok()?,
        Definition::Label(it) => it.parent(db).try_into().ok()?,
        // Items that appear in specific contexts.
        Definition::Field(it) => it.parent_def(db).into(),
        Definition::Variant(it) => Adt::Enum(it.parent_enum(db)).into(),
        Definition::GenericParam(it) => it.parent().into(),
        Definition::DeriveHelper(it) => it.derive().module(db).into(),
        Definition::InlineAsmOperand(it) => it.parent(db).try_into().ok()?,
        // Items that do not have parents and therefore will not have vnames.
        Definition::BuiltinAttr(_)
        | Definition::BuiltinType(_)
        | Definition::BuiltinLifetime(_)
        | Definition::TupleField(_)
        | Definition::ToolModule(_)
        | Definition::InlineAsmRegOrRegClass(_) => return None,
    };

    let mut anonymous = false;
    while let Definition::Module(module) = enclosing {
        match resolve_module(module, sema) {
            Some(def) => {
                enclosing = def;
                break;
            }
            None => {
                enclosing = Definition::Module(module.parent(sema.db).expect("root is resolvable"));
                anonymous = true
            }
        };
    }
    Some((enclosing, anonymous))
}

/// Construct a path segment for a given definition.
fn definition_segment(def: Definition, semantics: &Semantics<'_, RootDatabase>) -> Option<Segment> {
    let (identifier, vague_name) = match def {
        Definition::SelfType(_) => ("impl".into(), true),
        _ => match def.name(semantics.db) {
            None => ("definition".into(), true),
            Some(name) => (name.symbol().to_string(), false),
        },
    };

    let namespace = definition_namespace(def);
    let (parent, anonymous_scope) = parent_def(def, semantics)?;

    // We add position information in the path segment if the vname wouldn't otherwise be unique.
    let position = definition_to_syntax(def, semantics).and_then(|syntax| {
        // Locals can be shadowed within the same scope.
        let may_reuse_name = match def {
            Definition::Local(_) => true,
            Definition::Label(_) => true,
            Definition::Macro(mac) => !mac.is_macro_export(semantics.db),
            _ => false,
        };

        let needs_position = vague_name || anonymous_scope || may_reuse_name;
        needs_position.then(|| {
            // TODO(b/405361503): Original file range can collide -- we should encode the position
            // and macro expansion/file ID sequence instead.
            syntax
                .original_file_range_with_macro_call_body(semantics.db)
                .into_file_id(semantics.db)
                .into()
        })
    });

    Some(Segment { identifier, namespace, parent, position })
}

/// Names can belong to one of multiple distinct namespaces depending on what is being named.
/// See: https://doc.rust-lang.org/reference/names/namespaces.html
enum Namespace {
    Type,
    Value,
    Macro,
    Lifetime,
    Label,
    /// Technically, there is no explicit field namespace as fields can be accessed only through
    /// field expressions (https://doc.rust-lang.org/reference/names/namespaces.html#fields). We
    /// however want to create globally unique paths to fields without conflicts so will pretend
    /// that there is in fact a field namespace.
    Field,
}
// TODO(b/405352540): This namespace scheme is insufficient to prevent all possible vname conflicts.
// See comments of cl/730187665.

fn definition_namespace(def: Definition) -> Option<Namespace> {
    match def {
        Definition::Module(_)
        | Definition::Crate(_)
        | Definition::Adt(_)
        | Definition::Variant(_)
        | Definition::Trait(_)
        | Definition::TraitAlias(_)
        | Definition::TypeAlias(_)
        | Definition::GenericParam(GenericParam::TypeParam(_))
        | Definition::BuiltinType(_)
        | Definition::ExternCrateDecl(_) => {
            Some(Namespace::Type)
        }
        Definition::Function(_)
        | Definition::Const(_)
        | Definition::Static(_)
        | Definition::GenericParam(GenericParam::ConstParam(_))
        | Definition::Local(_) => {
            Some(Namespace::Value)
        }
        Definition::Macro(_) | Definition::DeriveHelper(_) | Definition::BuiltinAttr(_)
        | Definition::ToolModule(_) => {
            Some(Namespace::Macro)
        }
        Definition::GenericParam(GenericParam::LifetimeParam(_))
        | Definition::BuiltinLifetime(_) => {
            Some(Namespace::Lifetime)
        }
        Definition::Label(_) => {
            Some(Namespace::Label)
        }
        Definition::Field(_) | Definition::TupleField(_) => {
            Some(Namespace::Field)
        }
        // Impls are not named definitions.
        Definition::SelfType(_)
        // It is unclear which namespace these belong to, and we don't index inline asm regardless.
        | Definition::InlineAsmRegOrRegClass(_) | Definition::InlineAsmOperand(_) => {
            None
        }
    }
}

/// Map a definition to the syntax node that defined it.
///
/// This is implemented as a helper function rather than `impl ToSyntaxNode for Definition` as some
/// definitions (e.g., modules, locals) can be comprised of multiple separate syntax nodes. For
/// vname generation purposes picking just one syntax node for each definition is acceptable, but
/// for general indexing each syntax node probably needs to be handled explicitly.
fn definition_to_syntax(
    def: Definition,
    semantics: &Semantics<'_, RootDatabase>,
) -> Option<InFile<SyntaxNode>> {
    let to_syntax_node: &dyn ToSyntaxNode = match &def {
        Definition::Macro(hir_macro) => hir_macro,
        Definition::Field(hir_field) => hir_field,
        Definition::Module(hir_module) => {
            return hir_module
                .declaration_source(semantics.db)
                .map(|module| module.map(|ast_node| ast_node.syntax().clone()))
        }
        Definition::Function(hir_function) => hir_function,
        Definition::Adt(hir_adt) => hir_adt,
        Definition::Variant(hir_variant) => hir_variant,
        Definition::Const(hir_const) => hir_const,
        Definition::Static(hir_static) => hir_static,
        Definition::Trait(hir_trait) => hir_trait,
        Definition::TraitAlias(hir_trait_alias) => hir_trait_alias,
        Definition::TypeAlias(hir_type_alias) => hir_type_alias,
        Definition::SelfType(hir_impl) => hir_impl,
        Definition::GenericParam(hir_generic_param) => hir_generic_param,
        Definition::Local(hir_local) => &hir_local.primary_source(semantics.db),
        Definition::Label(hir_label) => hir_label,
        Definition::DeriveHelper(hir_derive_helper) => &hir_derive_helper.derive(),
        Definition::ExternCrateDecl(hir_extern_crate_decl) => hir_extern_crate_decl,
        Definition::InlineAsmOperand(hir_inline_asm_operand) => hir_inline_asm_operand,
        _ => return None,
    };

    to_syntax_node.to_in_file_syntax_node(semantics)
}
