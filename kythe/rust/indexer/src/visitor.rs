// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
use kythe::corpus::Corpus;
use kythe::schema::{Complete, EdgeKind, Fact, NodeKind, VName};
use kythe::writer::EntryWriter;
use rustc::hir;
use rustc::hir::{Block, Expr, ImplItem, ItemId, Pat};
use rustc::hir::def_id::DefId;
use rustc::hir::Expr_::{ExprCall, ExprLoop, ExprMatch, ExprMethodCall, ExprPath};
use rustc::hir::intravisit::*;
use rustc::hir::MatchSource::ForLoopDesugar;
use rustc::lint::{LateContext, LintContext};
use rustc::ty::{MethodCall, TyCtxt};
use syntax::ast;
use syntax::codemap::{CodeMap, Pos, Span, Spanned};

/// Indexes all kythe entries except files
pub struct KytheVisitor<'a, 'tcx: 'a> {
    pub writer: &'a Box<EntryWriter>,
    pub tcx: TyCtxt<'a, 'tcx, 'tcx>,
    pub codemap: &'a CodeMap,
    pub corpus: &'a Corpus,
    /// The vname of the parent item. The value changes as we recurse through the HIR, and is used
    /// for childof relationships
    parent_vname: Option<VName>,
}

impl<'a, 'tcx> KytheVisitor<'a, 'tcx> {
    /// Creates a new KytheVisitor
    pub fn new(writer: &'a Box<EntryWriter>,
               corpus: &'a Corpus,
               cx: &'a LateContext<'a, 'tcx>)
               -> KytheVisitor<'a, 'tcx> {
        KytheVisitor {
            writer: writer,
            tcx: cx.tcx,
            codemap: cx.sess().codemap(),
            corpus: corpus,
            parent_vname: None,
        }
    }

    /// Emits the appropriate node and facts for an anchor defined by a span
    /// and returns the node's VName
    fn anchor_from_span(&self, span: Span) -> VName {
        let start = self.codemap.lookup_byte_offset(span.lo);
        let end = self.codemap.lookup_byte_offset(span.hi);

        let start_byte = start.pos.to_usize();
        let end_byte = end.pos.to_usize();

        self.anchor(&start.fm.name, start_byte, end_byte)
    }

    /// Emits the appropriate node and facts for an anchor defined as a substring within a span
    /// and returns the node's VName
    fn anchor_from_sub_span(&self, span: Span, sub: &str) -> Result<VName, String> {
        let start = self.codemap.lookup_byte_offset(span.lo);
        let snippet = match self.codemap.span_to_snippet(span) {
            Ok(s) => s,
            Err(e) => return Err(format!("{:?}", e)),
        };
        let sub_start = match snippet.find(sub) {
            None => return Err(format!("Substring: '{}' not found in snippet '{}'", sub, snippet)),
            Some(s) => s,
        };
        let start_byte = start.pos.to_usize() + sub_start;
        let end_byte = start_byte + sub.len();

        Ok(self.anchor(&start.fm.name, start_byte, end_byte))
    }

    /// Emits an anchor node based on the byte range provided
    fn anchor(&self, file_name: &str, start_byte: usize, end_byte: usize) -> VName {
        let vname = self.corpus.anchor_vname(file_name, start_byte, end_byte);

        let start_str: String = start_byte.to_string();
        let end_str: String = end_byte.to_string();
        let file_vname = self.corpus.file_vname(&file_name);

        self.writer.node(&vname, Fact::NodeKind, &NodeKind::Anchor);
        self.writer.node(&vname, Fact::LocStart, &start_str);
        self.writer.node(&vname, Fact::LocEnd, &end_str);
        self.writer.edge(&vname, EdgeKind::ChildOf, &file_vname);
        vname
    }

    /// Produces a vname unique to a given DefId
    fn vname_from_defid(&self, def_id: DefId) -> VName {
        let def_id_num = def_id.index.as_u32();
        let var_name = self.tcx.absolute_item_path_str(def_id);
        self.corpus.def_vname(&var_name, def_id_num)
    }

    /// Emits the appropriate ref/call and childof nodes for a function call
    fn function_call(&self, call_node_id: ast::NodeId, callee_def_id: DefId) {
        let call_span = self.tcx.map.span(call_node_id);
        // The call anchor includes the subject (if function is a method) and the params
        let call_anchor_vname = self.anchor_from_span(call_span);
        let callee_vname = self.vname_from_defid(callee_def_id);

        self.writer.edge(&call_anchor_vname, EdgeKind::RefCall, &callee_vname);
        if let Some(ref parent_vname) = self.parent_vname {
            self.writer.edge(&call_anchor_vname, EdgeKind::ChildOf, &parent_vname);
        }
    }
}

/// Tests whether a span is the result of macro expansion
fn is_from_macro(span: &Span) -> bool {
    span.expn_id.into_u32() != u32::max_value()
}

impl<'v, 'tcx: 'v> Visitor<'v> for KytheVisitor<'v, 'tcx> {
    /// Enables recursing through nested items
    fn visit_nested_item(&mut self, id: ItemId) {
        let item = self.tcx.map.expect_item(id.id);
        self.visit_item(item);
    }

    /// Captures variable bindings
    fn visit_pat(&mut self, pat: &'v Pat) {
        use rustc::hir::PatKind::Binding;
        if let Binding(_, _, _) = pat.node {
            if let Some(def) = self.tcx.expect_def_or_none(pat.id) {
                let local_vname = self.vname_from_defid(def.def_id());
                let anchor_vname = self.anchor_from_span(pat.span);
                self.writer.edge(&anchor_vname, EdgeKind::DefinesBinding, &local_vname);
                self.writer.node(&local_vname, Fact::NodeKind, &NodeKind::Variable);
            }
        }
        walk_pat(self, pat);
    }

    /// Navigates the visitor around desugared for loops while hitting their important
    /// components
    fn visit_block(&mut self, block: &'v Block) {
        // Desugaring for loops turns:
        // [opt_ident]: for <pat> in <head> {
        //  <body>
        // }
        //
        // into
        //
        // {
        //   let result = match ::std::iter::IntoIterator::into_iter(<head>) {
        //     mut iter => {
        //       [opt_ident]: loop {
        //         match ::std::iter::Iterator::next(&mut iter) {
        //           ::std::option::Option::Some(<pat>) => <body>,
        //           ::std::option::Option::None => break
        //         }
        //       }
        //     }
        //   };
        //   result
        // }
        //
        // Here we check block contents to pick out <pat> <head> and <body>
        use rustc::hir::Decl_::DeclLocal;
        use rustc::hir::Stmt_::StmtDecl;
        use rustc::hir::PatKind::TupleStruct;
        if let [Spanned { node: StmtDecl(ref decl, _), .. }] = *block.stmts {
            if let DeclLocal(ref local) = decl.node {
                if let Some(ref expr) = local.init {
                    if let ExprMatch(ref base, ref outer_arms, ForLoopDesugar) = expr.node {
                        if let ExprCall(_, ref args) = base.node {
                            if let ExprLoop(ref block, _) = outer_arms[0].body.node {
                                if let Some(ref expr) = block.expr {
                                    if let ExprMatch(_, ref arms, _) = expr.node {
                                        if let TupleStruct(_, ref pats, _) = arms[0].pats[0].node {
                                            // Walk the interesting parts of <head>
                                            for a in args {
                                                self.visit_expr(&a);
                                            }
                                            // Walk <pat>
                                            self.visit_pat(&pats[0]);
                                            // Walk <body>
                                            self.visit_expr(&arms[0].body);
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        walk_block(self, block);
    }

    /// Captures refs and ref/calls
    fn visit_expr(&mut self, expr: &'v Expr) {
        if is_from_macro(&expr.span) {
            return walk_expr(self, expr);
        }

        match expr.node {
            // Paths are static references to items (including static methods)
            ExprPath(..) => {
                if let Some(def) = self.tcx.expect_def_or_none(expr.id) {
                    let def_id = def.def_id();
                    let local_vname = self.vname_from_defid(def_id);
                    let anchor_vname = self.anchor_from_span(expr.span);
                    self.writer.edge(&anchor_vname, EdgeKind::Ref, &local_vname);
                }
            }

            // Method calls are calls to any impl_fn that consumes self (requiring a vtable)
            ExprMethodCall(sp_name, _, _) => {
                let callee = self.tcx.tables.borrow().method_map[&MethodCall::expr(expr.id)];
                let local_vname = self.vname_from_defid(callee.def_id);
                let anchor_vname = self.anchor_from_span(sp_name.span);
                self.function_call(expr.id, callee.def_id);
                self.writer.edge(&anchor_vname, EdgeKind::Ref, &local_vname);
            }

            // Calls to statically addressable functions. The ref edge is handled in the ExprPath
            // branch
            ExprCall(ref fn_expr, _) => {
                let callee = self.tcx.def_map.borrow()[&fn_expr.id].base_def;
                self.function_call(expr.id, callee.def_id());
            }
            _ => (),
        }

        walk_expr(self, expr);
    }


    /// Captures function and method decl/bindings
    fn visit_fn(&mut self,
                kind: hir::intravisit::FnKind<'v>,
                decl: &'v hir::FnDecl,
                body: &'v hir::Block,
                span: Span,
                id: ast::NodeId) {

        use rustc::hir::intravisit::FnKind;
        match kind {
            FnKind::ItemFn(n, _, _, _, _, _, _) |
            FnKind::Method(n, _, _, _) => {
                let fn_name = n.to_string();
                let def_id = self.tcx.map.get_parent_did(body.id);
                let fn_vname = self.vname_from_defid(def_id);
                let decl_vname = self.anchor_from_span(span);

                self.writer.node(&fn_vname, Fact::NodeKind, &NodeKind::Function);
                self.writer.node(&fn_vname, Fact::Complete, &Complete::Definition);
                self.writer.edge(&decl_vname, EdgeKind::Defines, &fn_vname);

                if let Ok(bind_vname) = self.anchor_from_sub_span(span, &fn_name) {
                    self.writer.edge(&bind_vname, EdgeKind::DefinesBinding, &fn_vname)
                }
            }
            _ => (),
        };

        walk_fn(self, kind, decl, body, span, id);
    }

    /// Called instead of visit_item for items inside an impl, this function sets the
    /// parent_vname to the impl's vname
    fn visit_impl_item(&mut self, impl_item: &'v ImplItem) {
        let old_parent = self.parent_vname.clone();
        let def_id = self.tcx.map.local_def_id(impl_item.id);

        self.parent_vname = Some(self.vname_from_defid(def_id));
        walk_impl_item(self, impl_item);
        self.parent_vname = old_parent;
    }

    /// Run on every module-level item. Currently we only capture static and const items.
    /// (Functions are handled in visit_fn)
    fn visit_item(&mut self, item: &'v hir::Item) {
        let old_parent = self.parent_vname.clone();

        let def_id = self.tcx.map.local_def_id(item.id);
        let def_name = item.name.to_string();
        let def_vname = self.vname_from_defid(def_id);

        use rustc::hir::Item_::*;
        match item.node {
            ItemStatic(..) | ItemConst(..) => {
                let kind = if let ItemStatic(..) = item.node {
                    NodeKind::Variable
                } else {
                    NodeKind::Constant
                };
                let anchor_vname = self.anchor_from_span(item.span);
                self.writer.node(&def_vname, Fact::NodeKind, &kind);
                self.writer.edge(&anchor_vname, EdgeKind::Defines, &def_vname);

                // Substring matching is suboptimal, but there doesn't appear to be an accessible
                // node or span for the item name
                if let Ok(bind_vname) = self.anchor_from_sub_span(item.span, &def_name) {
                    self.writer.edge(&bind_vname, EdgeKind::DefinesBinding, &def_vname)
                }
            }
            _ => (),
        }

        self.parent_vname = Some(self.vname_from_defid(def_id));
        walk_item(self, item);
        self.parent_vname = old_parent;
    }
}
