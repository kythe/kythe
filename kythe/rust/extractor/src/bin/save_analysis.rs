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

use anyhow::Result;
use std::path::PathBuf;

pub fn run_analysis(arguments: Vec<String>, output_dir: PathBuf) -> Result<()> {
    let rustc_arguments = generate_arguments(arguments, &output_dir)?;
    let _input_files = kythe_rust_extractor::generate_analysis(rustc_arguments, output_dir)
        .map_err(|_| anyhow!("Failed to generate save_analysis"))?;
    Ok(())
}

fn generate_arguments(arguments: Vec<String>, output_dir: &PathBuf) -> Result<Vec<String>> {
    let argument_position = arguments
        .iter()
        .position(|arg| arg == "--")
        .ok_or_else(|| anyhow!("Could not find the start of the rustc arguments"))?;

    // Keep the "--" argument and replace it with an empty string.
    // The first argument must be an empty string.
    let mut rustc_arguments = arguments.split_at(argument_position).1.to_vec();
    rustc_arguments[0] = String::from("");

    // Change the output to the temporary directory
    let outdir_position = rustc_arguments
        .iter()
        .position(|arg| arg.contains("--out-dir"))
        .ok_or_else(|| anyhow!("Could not find the output directory argument"))?;
    let outdir_str =
        output_dir.to_str().ok_or_else(|| anyhow!("Couldn't convert temporary path to string"))?;
    rustc_arguments[outdir_position] = format!("--out-dir={}", outdir_str);

    Ok(rustc_arguments)
}
