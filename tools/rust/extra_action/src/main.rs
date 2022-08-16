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

use anyhow::{Context, Result};
use clap::Parser;
use extra_actions_base_rust_proto::*;
use protobuf::Message;
use std::fs::File;
use std::io::Write;
use std::path::PathBuf;

#[derive(Parser)]
#[clap(author = "The Kythe Authors")]
#[clap(about = "Rust Extra Action Generator", long_about = None)]
#[clap(rename_all = "snake_case")]
struct Args {
    /// Comma delimited source file paths
    #[clap(long, value_parser, value_name = "file,...")]
    src_files: String,

    /// Desired output path for the ExtraAction
    #[clap(long, value_parser)]
    output: PathBuf,

    /// The name for the crate
    #[clap(long, value_parser)]
    crate_name: String,

    /// The action owner
    #[clap(long, value_parser)]
    owner: String,

    /// The path to the Rust sysroot
    #[clap(long, value_parser)]
    sysroot: String,

    /// The path to the linker
    #[clap(long, value_parser)]
    linker: String,

    /// The path that $OUT_DIR will be set to
    #[clap(long, value_parser)]
    out_dir_env: Option<String>,

    /// Add \"--test\" to the compiler arguments
    #[clap(long, action)]
    test_lib: bool,
}

fn main() -> Result<()> {
    let args = Args::parse();

    let mut extra_action = ExtraActionInfo::new();
    extra_action.set_owner(args.owner);

    // Populate SpawnInfo
    let mut spawn_info = SpawnInfo::new();
    let source_files: Vec<String> = args.src_files.split(',').map(String::from).collect();
    let main_source_file = if source_files.len() == 1 {
        &source_files[0]
    } else {
        &source_files[source_files
            .iter()
            .position(|file_path| file_path.contains(&"main.rs") || file_path.contains(&"lib.rs"))
            .unwrap()]
    };
    let mut arguments: Vec<String> = vec![
        "--".to_string(),
        "rustc".to_string(),
        // Only the main source file gets passed to the compiler
        main_source_file.to_string(),
        format!("-L{}", args.sysroot),
        format!("--codegen=linker={}", args.linker),
        format!("--crate-name={}", &args.crate_name),
        // This path gets replaced by the extractor so it doesn't matter
        "--out-dir=/tmp".to_string(),
    ];
    if args.test_lib {
        arguments.push("--test".to_string());
    }
    spawn_info.set_argument(protobuf::RepeatedField::from_vec(arguments));
    spawn_info.set_input_file(protobuf::RepeatedField::from_vec(source_files));
    spawn_info.set_output_file(protobuf::RepeatedField::from_vec(vec![args.crate_name.clone()]));

    // If out_dir_env was set, set $OUT_DIR in the spawn info
    if let Some(out_dir) = args.out_dir_env {
        let mut env_var = EnvironmentVariable::new();
        env_var.set_name("OUT_DIR".to_string());
        env_var.set_value(out_dir);
        spawn_info.set_variable(protobuf::RepeatedField::from_vec(vec![env_var]));
    }

    // Add SpawnInfo extension to the ExtraActionInfo
    let action_unknown_fields = extra_action.mut_unknown_fields();
    // SpawnInfo is extension field 1003 on ExtraActionInfo
    // We have to use the `add_length_delimited` function but we only need to
    // pass the regulat bytes (not length delimited) of the SpawnInfo.
    let spawn_info_bytes: Vec<u8> = spawn_info
        .write_to_bytes()
        .with_context(|| "Failed to convert SpawnInfo to bytes".to_string())?;
    action_unknown_fields.add_length_delimited(1003u32, spawn_info_bytes);

    // Write the ExtraActionInfo to the file
    let extra_action_info_bytes: Vec<u8> = extra_action
        .write_to_bytes()
        .with_context(|| "Failed to convert ExtraActionInfo to bytes".to_string())?;
    let mut extra_action_file = File::create(args.output)
        .with_context(|| "Failed to create ExtraActionInfo file".to_string())?;
    extra_action_file
        .write_all(&extra_action_info_bytes)
        .with_context(|| "Failed to write ExtraActionInfo to file".to_string())?;

    Ok(())
}
