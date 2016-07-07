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
use kythe::schema::{VName, Fact, NodeKind, EdgeKind, Complete};
use kythe::writer::EntryWriter;
use rustc::lint::{LateContext, LintContext, LintPass, LateLintPass, LintArray};
use rustc::hir;
use std::io::prelude::*;
use std::io::stderr;
use std::fs::File;
use std::str;
use syntax::ast;
use syntax::codemap::{Pos, CodeMap, Span};

// Represents the shared data between all the check_* functions in rustc's LateLintPass
pub struct KytheLintPass {
    pub corpus: Corpus,
    pub writer: Box<EntryWriter>,
}

impl KytheLintPass {
    // Emits the appropriate node and facts for an anchor defined by a span
    // and returns the node's vname
    fn anchor_from_span(&self, span: Span, codemap: &CodeMap) -> VName {
        let start = codemap.lookup_byte_offset(span.lo);
        let end = codemap.lookup_byte_offset(span.hi);

        let start_byte = start.pos.to_usize();
        let end_byte = end.pos.to_usize();
        self.anchor(&start.fm.name, start_byte, end_byte)
    }
    // Emits the appropriate node and facts for an anchor defined as a substring within a span
    // and returns the node's VName.
    fn anchor_from_sub_span(&self,
                            span: Span,
                            sub: &str,
                            codemap: &CodeMap)
                            -> Result<VName, String> {
        let start = codemap.lookup_byte_offset(span.lo);
        let snippet = match codemap.span_to_snippet(span) {
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
}

// A no-op implementation since we aren't reporting linting errors
impl LintPass for KytheLintPass {
    fn get_lints(&self) -> LintArray {
        lint_array!()
    }
}

impl LateLintPass for KytheLintPass {
    fn check_crate(&mut self, cx: &LateContext, _: &hir::Crate) {
        for ref f in cx.sess().codemap().files.borrow().iter() {
            // The codemap contains references to virtual files all labeled <<std_macro>>
            // These are skipped as per this check
            if !f.is_real_file() {
                continue;
            }

            let vname = self.corpus.file_vname(&f.name);
            let mut contents = String::new();

            let res = File::open(&f.name).and_then(|mut f| f.read_to_string(&mut contents));

            match res {
                Err(e) => {
                    writeln!(stderr(), "Failed to read file {}\n{:?}", f.name, e).unwrap();
                }
                Ok(_) => {
                    self.writer.node(&vname, Fact::Text, &contents);
                    self.writer.node(&vname, Fact::NodeKind, &NodeKind::File);
                }
            }
        }
    }

    fn check_fn_post(&mut self,
                     ctx: &LateContext,
                     kind: hir::intravisit::FnKind,
                     _: &hir::FnDecl,
                     _: &hir::Block,
                     span: Span,
                     id: ast::NodeId) {
        match kind {
            // Module-level function definitions
            hir::intravisit::FnKind::ItemFn(n, _, _, _, _, _, _) => {
                let fn_name = n.to_string();
                let decl_vname = self.anchor_from_span(span, ctx.sess().codemap());
                let fn_path = ctx.tcx.node_path_str(id);
                let fn_vname = self.corpus.fn_vname(&fn_path);
                self.writer.node(&fn_vname, Fact::NodeKind, &NodeKind::Function);
                self.writer.node(&fn_vname, Fact::Complete, &Complete::Definition);
                self.writer.edge(&decl_vname, EdgeKind::Defines, &fn_vname);

                match self.anchor_from_sub_span(span, &fn_name, ctx.sess().codemap()) {
                    Ok(bind_vname) => {
                        self.writer.edge(&bind_vname, EdgeKind::DefinesBinding, &fn_vname)
                    }
                    // TODO(djrenren): use a logging library
                    Err(e) => writeln!(stderr(), "{}", e).unwrap(),
                }
            }
            _ => (),
        }
    }
}
