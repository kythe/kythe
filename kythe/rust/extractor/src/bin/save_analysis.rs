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

use anyhow::{anyhow, Context, Result};
use std::fs::File;
use std::io::{self, BufRead};
use std::path::{Path, PathBuf};

/// Generates a save analysis in `output_dir`
///
/// * `arguments` - The Bazel arguments extracted from the extra action protobuf
/// * `output_dir` - The base directory to output the save_analysis
pub fn generate_save_analysis(
    arguments: Vec<String>,
    output_dir: PathBuf,
    output_file_name: &str,
) -> Result<()> {
    let rustc_arguments = generate_arguments(arguments, &output_dir)?;
    kythe_rust_extractor::generate_analysis(rustc_arguments, output_dir, output_file_name)
        .map_err(|_| anyhow!("Failed to generate save_analysis"))?;
    Ok(())
}

/// Extracts the Rust compiler arguments from `arguments` and changes the
/// compiler output directory to `output_dir`
///
/// * `arguments` - The Bazel arguments extracted from the extra action protobuf
/// * `output_dir` - The directory to output the binary produced by the Rust
///   compiler
fn generate_arguments(arguments: Vec<String>, output_dir: &Path) -> Result<Vec<String>> {
    let argument_position = arguments
        .iter()
        .position(|arg| arg == "--")
        .ok_or_else(|| anyhow!("Could not find the start of the rustc arguments"))?;

    // The rust compiler executable path is the argument directly after "--"
    // We keep the path argument and  replace it with an empty string because
    // `kythe_rust_extractor::generate_analysis` requires the first argument in
    // `arguments` to be an empty string
    let mut rustc_arguments = arguments.split_at(argument_position + 1).1.to_vec();

    rustc_arguments[0] = String::from("");

    // Change the original compiler output to the temporary directory
    let outdir_position = rustc_arguments
        .iter()
        .position(|arg| arg.contains("--out-dir"))
        .ok_or_else(|| anyhow!("Could not find the output directory argument: {:?}", arguments))?;
    let outdir_str =
        output_dir.to_str().ok_or_else(|| anyhow!("Couldn't convert temporary path to string"))?;
    rustc_arguments[outdir_position] = format!("--out-dir={}", outdir_str);

    // Check if there is a file containing environment variables
    let env_file_position = arguments.iter().position(|arg| arg == "--env-file");
    if let Some(position) = env_file_position {
        let env_file = &arguments[position + 1];
        let path = Path::new(env_file);
        process_env_file(path).context("Failed to process wrapper environment file")?;
    }

    // Check if there is a file containing extra compiler arguments
    let arg_file_position = arguments.iter().position(|arg| arg == "--arg-file");
    if let Some(position) = arg_file_position {
        let arg_file = &arguments[position + 1];
        let path = Path::new(arg_file);
        let new_arguments =
            process_arg_file(path).context("Failed to process wrapper argument file")?;
        rustc_arguments.extend(new_arguments);
    }

    Ok(rustc_arguments)
}

/// Process an environment variable file passed to the process wrapper
///
/// * `path` - The path of the environment variable file
fn process_env_file(path: &Path) -> Result<()> {
    let pwd = std::env::current_dir().context("Couldn't determine pwd")?;
    for line in io::BufReader::new(File::open(path)?).lines() {
        let line_string = line.context("Failed to read line of env file")?;
        let split: Vec<&str> = line_string.trim().split('=').collect();
        let value = split[1].replace("${pwd}", pwd.to_str().unwrap());
        std::env::set_var(split[0], value);
    }
    Ok(())
}

/// Process a compiler argument file passed to the process wrapper
///
/// * `path` - The path of the compiler argument file
fn process_arg_file(path: &Path) -> Result<Vec<String>> {
    io::BufReader::new(File::open(path)?)
        .lines()
        .map(|line| line.context("Failed to read line of arg file"))
        .collect()
}
