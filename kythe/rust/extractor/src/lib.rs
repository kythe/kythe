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

mod callback;

extern crate rustc_driver;
extern crate rustc_errors;
extern crate rustc_interface;
extern crate rustc_save_analysis;
extern crate rustc_session;

use callback::CallbackShim;
use rustc_driver::run_compiler;
use std::path::PathBuf;

pub fn generate_analysis(
    rustc_arguments: Vec<String>,
    output_dir: PathBuf,
) -> Result<(), ()> {
    let mut callback_shim = CallbackShim::new(output_dir);

    std::env::set_var(
        "RUST_SAVE_ANALYSIS_CONFIG",
        "{\"output_file\":null,\"full_docs\":true,\"pub_only\":false,\"reachable_only\":false,\"distro_crate\":false,\"signatures\":false,\"borrow_data\":false}"
    );

    rustc_driver::install_ice_hook();
    rustc_driver::catch_fatal_errors(|| {
        run_compiler(
            // Arguments passed to the rust compiler. The first argument must be an empty string
            &rustc_arguments,
            // Handle compiler callbacks to run save analysis and capture input files
            &mut callback_shim,
            None,
            None,
        )
    })
    .map(|_| ())
    .map_err(|_| ())?;

    Ok(())
}
