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
use kythe::schema::{Fact, NodeKind};
use kythe::writer::EntryWriter;
use rustc::lint::{LateContext, LintContext, LintPass, LateLintPass, LintArray};
use rustc::hir::intravisit::Visitor;
use rustc::hir;
use syntax::ast;
use visitor::KytheVisitor;

/// Represents the data that we pass to the KytheVisitor when our LintPass is invoked
pub struct KytheLintPass {
    corpus: Corpus,
    writer: Box<EntryWriter>,
}

impl KytheLintPass {
    /// A convenience constructor
    pub fn new(corpus: Corpus, writer: Box<EntryWriter>) -> KytheLintPass {
        KytheLintPass {
            corpus: corpus,
            writer: writer,
        }
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
        let codemap = cx.sess().codemap();

        // Index all the file nodes
        for ref f in codemap.files.borrow().iter() {
            // The codemap contains references to virtual files all labeled <<std_macro>>
            // These are skipped as per this check
            if !f.is_real_file() {
                continue;
            }

            // References to the core crate are filtered out here
            if let Some(ref content) = f.src {
                let vname = self.corpus.file_vname(&f.name);

                self.writer.node(&vname, Fact::NodeKind, &NodeKind::File);
                self.writer.node(&vname, Fact::Text, content);
            }
        }

        // Run our visitor which will index everything but files
        let mut vis = KytheVisitor::new(&self.writer, &self.corpus, cx);
        vis.visit_mod(&cx.krate.module, cx.krate.module.inner, ast::DUMMY_NODE_ID);
    }
}
