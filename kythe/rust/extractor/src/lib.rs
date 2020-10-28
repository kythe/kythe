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
#![feature(rustc_private)]

extern crate rustc_driver;
extern crate rustc_interface;
extern crate rustc_save_analysis;
extern crate rustc_session;

use rustc_driver::run_compiler;
use rustc_driver::{Callbacks, Compilation};
use rustc_interface::{interface, Queries};
use rustc_save_analysis::DumpHandler;
use std::path::PathBuf;

/// Generate a save_analysis in `output_dir`
///
/// `rustc_arguments` is a `Vec<String>` containing Rust compiler arguments. The
/// first element must be an empty string.
/// The save_analysis JSON output file will be located at
/// {output_dir}/save-analysis/{crate_name}.json
pub fn generate_analysis(rustc_arguments: Vec<String>, output_dir: PathBuf) -> Result<(), String> {
    let first_arg =
        rustc_arguments.get(0).ok_or_else(|| "Arguments vector should not be empty".to_string())?;
    if first_arg != &"".to_string() {
        return Err("The first argument must be an empty string".into());
    }

    let mut callback_shim = CallbackShim::new(output_dir);

    rustc_driver::catch_fatal_errors(|| {
        run_compiler(&rustc_arguments, &mut callback_shim, None, None)
    })
    .map(|_| ())
    .map_err(|_| "A compiler error occurred".to_string())?;

    Ok(())
}

/// Handles compiler callbacks to enable and dump the save_analysis
#[derive(Default)]
struct CallbackShim {
    output_dir: PathBuf,
}

impl CallbackShim {
    /// Create a new CallbackShim that dumps save_analysis files to `output_dir`
    pub fn new(output_dir: PathBuf) -> Self {
        Self { output_dir }
    }
}

impl Callbacks for CallbackShim {
    // Always enable save_analysis generation
    fn config(&mut self, config: &mut interface::Config) {
        config.opts.debugging_opts.save_analysis = true;
    }

    fn after_analysis<'tcx>(
        &mut self,
        compiler: &interface::Compiler,
        queries: &'tcx Queries<'tcx>,
    ) -> Compilation {
        let input = compiler.input();
        let crate_name = queries.crate_name().unwrap().peek().clone();

        // Configure the save_analysis to include full documentation.
        // Normally this would be set using a `rls_data::config::Config` struct on the
        // fourth parameter of `process_crate`. However, the Rust compiler
        // falsely claims that there is a mismatch between rustc_save_analysis's
        // `rls_data::config::Config` and ours, even though we use the same version.
        // This forces us to use the environment variable method of configuration
        // instead.
        std::env::set_var(
            "RUST_SAVE_ANALYSIS_CONFIG",
            r#"{"output_file":null,"full_docs":true,"pub_only":false,"reachable_only":false,"distro_crate":false,"signatures":false,"borrow_data":false}"#,
        );

        // Perform the save_analysis and dump it to the directory
        // The JSON file is saved at {self.output_dir}/save-analysis/{crate_name}.json
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
