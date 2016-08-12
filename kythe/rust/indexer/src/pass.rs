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
use kythe::writer::EntryWriter;
use rustc::lint::{LateContext, LintPass, LateLintPass, LintArray};
use rustc::hir::intravisit::Visitor;
use rustc::hir;
use syntax::ast;
use visitor::KytheVisitor;

/// Represents the data that we pass to the KytheVisitor when our LintPass is invoked
pub struct KytheLintPass {
    writer: Box<EntryWriter>,
}

impl KytheLintPass {
    /// A convenience constructor
    pub fn new(writer: Box<EntryWriter>) -> KytheLintPass {
        KytheLintPass { writer: writer }
    }
}

// A no-op implementation since we aren't reporting linting errors
impl LintPass for KytheLintPass {
    fn get_lints(&self) -> LintArray {
        lint_array!()
    }
}

impl<'a> LateLintPass for KytheLintPass {
    fn check_crate(&mut self, cx: &LateContext, _: &hir::Crate) {
        let mut vis = KytheVisitor::new(&self.writer, cx);
        vis.index_files();
        vis.visit_mod(&cx.krate.module, cx.krate.module.inner, ast::DUMMY_NODE_ID);
    }
}
