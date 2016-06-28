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

use kythe::Corpus;
use rustc::lint::{LateContext, LintContext, LintPass, LateLintPass, LintArray};
use rustc::hir;
use std::io::{stderr, Write};

// Represents the shared data between all the check_* functions
pub struct Pass {
    pub corpus: Corpus,
}

// A no-op implementation since we aren't reporting linting errors
impl LintPass for Pass {
    fn get_lints(&self) -> LintArray {
        lint_array!()
    }
}

impl LateLintPass for Pass {
    fn check_crate(&mut self, cx: &LateContext, _: &hir::Crate) {
        for ref f in cx.sess().codemap().files.borrow().iter() {
            // The codemap contains references to virtual files all labeled <<std_macro>>
            // These are skipped as per this check
            if !f.is_real_file() {
                continue;
            }
            // If the file can't be read the indexer will crash here
            // and print the appropriate message
            let entries_res = self.corpus.file_node(&f.name);
            match entries_res {
                Err(e) => {
                    let mut stderr = stderr();
                    writeln!(&mut stderr, "Failed to read file {}\n{:?}", f.name, e).unwrap();
                }
                _ => (),
            }
        }
    }
}
