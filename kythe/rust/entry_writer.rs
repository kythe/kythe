// Deps:
//   :file_range
//   :vnames
//   kythe:common_rust_proto
//   kythe:storage_rust_proto
//   protobuf:protobuf
//   rust_analyzer

use common_rust_proto::MarkedSourceView;
use file_range::FileRange;
use protobuf::{proto, Serialize};
use rust_analyzer::{
    hir::{InFile, InRealFile},
    ide::{RootDatabase, Semantics},
    ide_db::defs::Definition,
    syntax::{ast, AstNode, NodeOrToken, SyntaxElement, SyntaxKind, SyntaxNode},
    tt::TextRange,
};
use storage_rust_proto::{Entry, EntryView, VName};
use vnames::VNameFactory;

#[derive(Debug, Clone, Copy)]
pub enum EdgeKind {
    ChildOf,
    CompletedBy,
    Defines,
    DefinesBinding,
    DefinesImplicit,
    Documents,
    Extends,
    Overrides,
    Param(usize),
    Ref,
    Tagged,
    TypeParam(usize),
}

impl std::fmt::Display for EdgeKind {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            EdgeKind::ChildOf => write!(f, "childof"),
            EdgeKind::CompletedBy => write!(f, "completedby"),
            EdgeKind::Defines => write!(f, "defines"),
            EdgeKind::DefinesBinding => write!(f, "defines/binding"),
            EdgeKind::DefinesImplicit => write!(f, "defines/implicit"),
            EdgeKind::Documents => write!(f, "documents"),
            EdgeKind::Extends => write!(f, "extends"),
            EdgeKind::Overrides => write!(f, "overrides"),
            EdgeKind::Param(index) => write!(f, "param.{index}"),
            EdgeKind::Ref => write!(f, "ref"),
            EdgeKind::Tagged => write!(f, "tagged"),
            EdgeKind::TypeParam(index) => write!(f, "tparam.{index}"),
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub enum NodeKind {
    Anchor,
    Constant,
    Diagnostic,
    Doc,
    File,
    Interface,
    Function,
    Macro,
    Package,
    Record,
    Sum,
    TypeAlias,
    TypeVariable,
    Variable,
}

impl From<NodeKind> for &'static str {
    fn from(value: NodeKind) -> &'static str {
        match value {
            NodeKind::Anchor => "anchor",
            NodeKind::Constant => "constant",
            NodeKind::Diagnostic => "diagnostic",
            NodeKind::Doc => "doc",
            NodeKind::File => "file",
            NodeKind::Interface => "interface",
            NodeKind::Function => "function",
            NodeKind::Macro => "macro",
            NodeKind::Package => "package",
            NodeKind::Record => "record",
            NodeKind::Sum => "sum",
            NodeKind::TypeAlias => "talias",
            NodeKind::TypeVariable => "tvar",
            NodeKind::Variable => "variable",
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub enum NodeSubkind {
    Field,
    Struct,
    Union,
    Enum,
}

impl From<NodeSubkind> for &'static str {
    fn from(value: NodeSubkind) -> &'static str {
        match value {
            NodeSubkind::Field => "field",
            NodeSubkind::Struct => "struct",
            NodeSubkind::Union => "union",
            NodeSubkind::Enum => "enum",
        }
    }
}

/// Wrapper around a callback that emits Kythe entries. This exists for both convenience and to
/// restrict the scope of each `Indexable` implementation (by generally only allowing semantic
/// edges 'outwards' and anchor edges 'inwards').
pub struct EntryWriter<'a> {
    /// The vname of the Rust entity we want to model in Kythe.
    pub target: VName,
    /// Callback to emit a Kythe entry.
    write: &'a mut dyn FnMut(EntryView),
    vnames: &'a VNameFactory<'a>,
    sema: &'a Semantics<'a, RootDatabase>,
}

impl<'a> EntryWriter<'a> {
    pub fn new(
        target: VName,
        write: &'a mut dyn FnMut(EntryView),
        vnames: &'a VNameFactory,
        sema: &'a Semantics<'a, RootDatabase>,
    ) -> EntryWriter<'a> {
        EntryWriter { target, write, vnames, sema }
    }

    pub fn kind(&mut self, kind: NodeKind, subkind: Option<NodeSubkind>) {
        self.fact(self.target.clone(), "node/kind", <&str>::from(kind).as_bytes());
        if let Some(subkind) = subkind {
            self.fact(self.target.clone(), "subkind", <&str>::from(subkind).as_bytes());
        }
    }

    pub fn text(&mut self, text: &[u8]) {
        self.fact(self.target.clone(), "text", text);
    }

    pub fn message(&mut self, text: &[u8]) {
        self.fact(self.target.clone(), "message", text);
    }

    pub fn complete(&mut self, complete: bool) {
        self.fact(
            self.target.clone(),
            "complete",
            if complete { b"complete" } else { b"incomplete" },
        );
    }

    pub fn loc(&mut self, start: u32, end: u32) {
        self.fact(self.target.clone(), "loc/start", start.to_string().as_bytes());
        self.fact(self.target.clone(), "loc/end", end.to_string().as_bytes());
    }

    pub fn deprecated(&mut self, msg: &str) {
        self.fact(self.target.clone(), "tag/deprecated", msg.as_bytes());
    }

    pub fn code(&mut self, marked_source: MarkedSourceView) {
        if let Ok(bytes) = marked_source.serialize() {
            self.fact(self.target.clone(), "code", &bytes);
        }
    }

    /// Output an edge towards another semantic node.
    pub fn edge_to(&mut self, kind: EdgeKind, to: Definition) {
        if let Ok(to) = self.vnames.definition(to) {
            self.edge(self.target.clone(), kind, to);
        }
    }

    /// Create an anchor that spans some syntax and output an edge from it.
    pub fn anchor(&mut self, kind: EdgeKind, syntax: InFile<SyntaxNode>) {
        let range = range_for_anchor(syntax);
        // If the syntax was expanded from a macro arg (e.g. `vec![xxx + yyy]`), then we use the
        // range in the original source.
        // This may be a zero-width range at the call site if there's no exact correspondence.
        self.range_anchor(kind, range_in_file(range, self.sema.db));
    }

    /// Create an anchor that spans the 'name' (as defined by a `ast::HasName` impl) sub-span of
    /// some syntax and output an edge from it.
    pub fn name_anchor(&mut self, kind: EdgeKind, syntax: InFile<SyntaxNode>) {
        let token = ast::AnyHasName::cast(syntax.value)
            .as_ref()
            .and_then(ast::HasName::name)
            .and_then(|name| name.ident_token().or_else(|| name.self_token()));
        if let Some(token) = token {
            let range =
                range_in_file(InFile::new(syntax.file_id, token.text_range()), self.sema.db);
            self.range_anchor(kind, range);
        }
    }

    /// Create an anchor and output an edge from it.
    pub fn range_anchor(&mut self, kind: EdgeKind, range: FileRange) {
        let anchor = self.vnames.anchor(range);
        self.edge(anchor.clone(), kind, self.target.clone());
        let mut anchor_write = EntryWriter::new(anchor, self.write, self.vnames, self.sema);
        anchor_write.kind(NodeKind::Anchor, None);
        anchor_write.loc(range.start, range.end);
    }

    fn edge(&mut self, from: VName, kind: EdgeKind, to: VName) {
        self.entry(proto!(Entry {
            source: from,
            edge_kind: format!("/kythe/edge/{kind}"),
            target: to,
            // Edges *must* set the fact kind to "/" for verifier tests to pass!
            fact_name: "/",
        }));
    }

    fn fact(&mut self, vname: VName, kind: &str, value: &[u8]) {
        self.entry(proto!(Entry {
            source: vname,
            fact_name: format!("/kythe/{kind}"),
            fact_value: value
        }))
    }

    fn entry(&mut self, entry: Entry) {
        (self.write)(entry.as_view());
    }
}

fn range_for_anchor(node: InFile<SyntaxNode>) -> InFile<TextRange> {
    let InFile { value: node, file_id } = node;
    let element = match node.kind() {
        SyntaxKind::RANGE_PAT
        | SyntaxKind::RANGE_EXPR
        | SyntaxKind::TRY_EXPR
        | SyntaxKind::BIN_EXPR
        | SyntaxKind::PREFIX_EXPR => node.children_with_tokens().find(|c| c.kind().is_punct()),
        // For a[b], we don't have a useful contiguous range for the operator, so link `[`.
        SyntaxKind::INDEX_EXPR => {
            node.children_with_tokens().find(|c| c.kind() == SyntaxKind::L_BRACK)
        }
        SyntaxKind::AWAIT_EXPR => {
            node.children_with_tokens().find(|c| c.kind() == SyntaxKind::AWAIT_KW)
        }
        _ => None,
    }
    .unwrap_or_else(|| SyntaxElement::Node(node.clone()));

    match element {
        NodeOrToken::Token(tok) => InFile::new(file_id, tok.text_range()),
        NodeOrToken::Node(node) => {
            // We exclude any preceding comment or whitespace tokens from the range of the
            // anchor node we're going to create.
            let begin = node
                .children_with_tokens()
                .find(|child| {
                    child.kind() != SyntaxKind::COMMENT && child.kind() != SyntaxKind::WHITESPACE
                })
                .unwrap_or_else(|| SyntaxElement::Node(node.clone()));
            InFile::new(
                file_id,
                TextRange::new(begin.text_range().start(), node.text_range().end()),
            )
        }
    }
}

// Find the original source code for a (possibly macro-expanded) range of syntax.
//
// If the syntax was expanded from a macro argument, return the range in that argument.
//
//    vec![a + b]    =>    { v = Vec::new(); v.push(a + b); v }
//         ~                                        ~
//          \______________________________________/
//
// If expanded from a macro body, returns an zero-width anchor at the start of the macro call.
//
//    vec![a + b]    =>    { v = Vec::new(); v.push(a + b); v }
//    ^                          ~~~
//     \________________________/
//
// This is in contrast to original_file_range(), which may fall back to returning the range within
// the macro body, or the macro call itself.
fn range_in_file(mut range: InFile<TextRange>, db: &RootDatabase) -> FileRange {
    while let Some(macro_file) = range.file_id.macro_file() {
        // Range in calling macro's args, if that's where this code came from.
        let arg_range = (|| {
            let expn = macro_file.expansion_info(db);
            let arg = InFile::new(expn.call_file(), expn.arg().value?.text_range());
            let origins = expn.map_range_up_once(db, range.value);
            let [mut origin_range] = *origins.value else {
                return None; // Not expanded from a single contiguous range.
            };
            // map_range_up_once will expand the range to cover whole tokens.
            // If we had a zero-length range, restore that property.
            // See `refr!(refr_y!())` in macro_expn_test.rs: `map_range_up_once` transforms the zero
            // size range ^here in refr!s expansion to cover the `refr_y` token in the file.
            if range.value.is_empty() {
                origin_range = TextRange::empty(origin_range.start());
            }
            let range = InFile::new(origins.file_id, origin_range);
            intersects(arg, range).then_some(range)
        })();
        // Otherwise, a zero-width anchor at the beginning of the macro call.
        range = arg_range.unwrap_or_else(|| {
            macro_file.loc(db).to_node(db).map(|node| TextRange::empty(node.text_range().start()))
        });
    }
    let range = range.into_real_file().unwrap();
    FileRange::new(range.file_id.file_id(db), range.value)
}

fn intersects(a: InFile<TextRange>, b: InFile<TextRange>) -> bool {
    a.file_id == b.file_id && a.value.intersect(b.value).is_some()
}
