// Deps:
//   crates.io:either
//   rust_analyzer

use either::Either;
use rust_analyzer::{
    hir::{
        Adt, Const, ExternCrateDecl, Field, Function, GenericParam, HasSource, Impl, InFile,
        InlineAsmOperand, Label, LocalSource, Macro, SelfParam, Semantics, Static, Trait,
        TraitAlias, TypeAlias, TypeOrConstParam, Variant,
    },
    ide::RootDatabase,
    syntax::{AstNode, SyntaxNode},
};

/// Trait that facilitates conversion from HIR nodes to (untyped) syntax nodes.
pub trait ToSyntaxNode {
    /// Convert to a syntax node that is in some HIR-representation of a file (so potentially
    /// inside a macro rather than a concrete source file).
    fn to_in_file_syntax_node(
        &self,
        semantics: &Semantics<'_, RootDatabase>,
    ) -> Option<InFile<SyntaxNode>>;
}

// We need an additional marker trait for the generic implementation of `ToSyntaxNode` below to
// compile without a conflicting trait implementation error.
trait Marker {}
impl Marker for Macro {}
impl Marker for Field {}
impl Marker for Function {}
impl Marker for Adt {}
impl Marker for Variant {}
impl Marker for Const {}
impl Marker for Static {}
impl Marker for Trait {}
impl Marker for TraitAlias {}
impl Marker for TypeAlias {}
impl Marker for Impl {}
impl Marker for Label {}
impl Marker for ExternCrateDecl {}
impl Marker for InlineAsmOperand {}
impl Marker for SelfParam {}

// We can use rust-analyzer's `HasSource` to implement `ToSyntaxNode` on most (but not all) types we
// want to convert.
impl<T: Copy + HasSource + Marker> ToSyntaxNode for T
where
    <T as HasSource>::Ast: AstNode,
{
    fn to_in_file_syntax_node(
        &self,
        semantics: &Semantics<'_, RootDatabase>,
    ) -> Option<InFile<SyntaxNode>> {
        semantics.source(*self).map(|ast_node| ast_node.syntax().cloned())
    }
}

impl ToSyntaxNode for GenericParam {
    fn to_in_file_syntax_node(
        &self,
        semantics: &Semantics<'_, RootDatabase>,
    ) -> Option<InFile<SyntaxNode>> {
        let map_type_or_const_param = |param: TypeOrConstParam| {
            let in_file = semantics.source(param)?;
            let value = match in_file.value {
                Either::Left(type_or_const_param) => type_or_const_param.syntax().clone(),
                Either::Right(trait_or_alias) => trait_or_alias.syntax().clone(),
            };
            Some(InFile::new(in_file.file_id, value))
        };

        match *self {
            GenericParam::TypeParam(param) => map_type_or_const_param(param.merge()),
            GenericParam::ConstParam(param) => map_type_or_const_param(param.merge()),
            GenericParam::LifetimeParam(param) => {
                semantics.source(param).map(|ast_node| ast_node.syntax().cloned())
            }
        }
    }
}

// A single `hir::Local` can have multiple sources (e.g., in pattern matching) so we shouldn't
// implement `ToSyntaxNode` directly on it. We can implement it for a particular source though.
impl ToSyntaxNode for LocalSource {
    fn to_in_file_syntax_node(
        &self,
        _semantics: &Semantics<'_, RootDatabase>,
    ) -> Option<InFile<SyntaxNode>> {
        Some(self.source.as_ref().map(|either| match either {
            Either::Left(ident_pat) => ident_pat.syntax().clone(),
            Either::Right(self_param) => self_param.syntax().clone(),
        }))
    }
}
