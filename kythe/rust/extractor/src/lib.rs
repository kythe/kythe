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

#[macro_use]
extern crate anyhow;

extern crate rustc_error_codes;
extern crate rustc_errors;
extern crate rustc_interface;
extern crate rustc_save_analysis;
extern crate rustc_session;

use anyhow::{Context, Result};
use rustc_errors::registry::Registry;
use rustc_interface::interface;
use rustc_save_analysis::DumpHandler;
use rustc_session::config::{self, Input};
use rustc_session::{getopts, DiagnosticOutput};
use std::path::PathBuf;

/// Generate a save_analysis in `output_dir`
///
/// `rustc_arguments` is a `Vec<String>` containing Rust compiler arguments. The
/// first element must be an empty string.
/// The save_analysis JSON output file will be located at
/// {output_dir}/save-analysis/{crate_name}.json
pub fn generate_analysis(rustc_arguments: Vec<String>, output_dir: PathBuf) -> Result<()> {
    let first_arg =
        rustc_arguments.get(0).ok_or_else(|| anyhow!("Arguments vector should not be empty"))?;
    if first_arg != &"".to_string() {
        return Err(anyhow!("The first argument must be an empty string"));
    }

    // Generate the configuration to parse the options
    let mut options = getopts::Options::new();
    for option in config::rustc_optgroups() {
        (option.apply)(&mut options);
    }
    let matches = options.parse(&rustc_arguments[1..]).context("Failed to parse options")?;

    // Create the session options
    let sopts = config::build_session_options(&matches);

    // Make PathBufs for the output directory and file name
    let odir = matches.opt_str("out-dir").map(|o| PathBuf::from(&o));
    let ofile = matches.opt_str("o").map(|o| PathBuf::from(&o));

    // Process configuration flags
    let cfg = interface::parse_cfgspecs(matches.opt_strs("cfg"));

    // Create PathBufs for the input file
    let ifile = &matches.free[0];

    // Create the compilation configuration
    let config = interface::Config {
        opts: sopts,
        crate_cfg: cfg,
        input: Input::File(PathBuf::from(ifile)),
        input_path: Some(PathBuf::from(ifile)),
        output_file: ofile,
        output_dir: odir,
        file_loader: None,
        diagnostic_output: DiagnosticOutput::Default,
        stderr: None,
        lint_caps: Default::default(),
        parse_sess_created: None,
        register_lints: None,
        override_queries: None,
        make_codegen_backend: None,
        registry: Registry::new(&rustc_error_codes::DIAGNOSTICS),
    };

    interface::run_compiler(config, |compiler| {
        compiler.enter(|queries| -> Result<()> {
            queries.parse().map_err(|_| anyhow!("Failed to parse code"))?;
            queries.expansion().map_err(|_| anyhow!("Failed to expand code"))?;
            queries.prepare_outputs().map_err(|_| anyhow!("Failed to prepate outputs"))?;
            queries.global_ctxt().map_err(|_| anyhow!("Failed to generate global context"))?;

            std::env::set_var(
                "RUST_SAVE_ANALYSIS_CONFIG",
                r#"{"output_file":null,"full_docs":true,"pub_only":false,"reachable_only":false,"distro_crate":false,"signatures":false,"borrow_data":false}"#,
            );

            queries
            .global_ctxt().map_err(|_| anyhow!("Failed to retrieve global context"))?.peek_mut().enter(|tcx| {
                let input = compiler.input();
                let crate_name = queries.crate_name().unwrap().peek().clone();
                rustc_save_analysis::process_crate(
                    tcx,
                    &crate_name,
                    &input,
                    None,
                    DumpHandler::new(Some(output_dir.as_path()), &crate_name),
                );
            });

            Ok(())
        })?;

        Ok(())
    })
    .map(|_| ())
}
