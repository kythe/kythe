// Copyright 2020 The Kythe Authors. All rights reserved.
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

use rustc_driver::{Callbacks, Compilation};
use rustc_interface::{interface, Queries};
use rustc_save_analysis::DumpHandler;
use std::path::PathBuf;

#[derive(Default)]
pub struct CallbackShim {
    output_dir: PathBuf,
}

impl Callbacks for CallbackShim {
    fn config(&mut self, config: &mut interface::Config) {
        config.opts.debugging_opts.save_analysis = true;
    }

    fn after_expansion<'tcx>(
        &mut self,
        _compiler: &interface::Compiler,
        _queries: &'tcx Queries<'tcx>,
    ) -> Compilation {
        Compilation::Continue
    }

    fn after_analysis<'tcx>(
        &mut self,
        compiler: &interface::Compiler,
        queries: &'tcx Queries<'tcx>,
    ) -> Compilation {
        let input = compiler.input();
        let crate_name = queries.crate_name().unwrap().peek().clone();

        // TODO: Determine how to change save-analysis config to emit full_docs
        queries.global_ctxt().unwrap().peek_mut().enter(|tcx| {
            rustc_save_analysis::process_crate(
                tcx,
                &crate_name,
                &input,
                None,
                DumpHandler::new(Some(self.output_dir.as_path()), &crate_name),
            )
        });

        Compilation::Continue
    }
}

impl CallbackShim {
    pub fn new(output_dir: PathBuf) -> Self {
        Self { output_dir }
    }
}
