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
use clap::{App, Arg};
use extra_actions_base_rust_proto::*;
use protobuf::Message;
use std::fs::File;
use std::io::Write;
use std::path::Path;

fn main() -> Result<()> {
    let matches = App::new("Kythe Rust Extractor")
        .arg(
            Arg::with_name("src_files")
                .long("src_files")
                .required(true)
                .takes_value(true)
                .help("Comma delimited source file paths"),
        )
        .arg(
            Arg::with_name("output")
                .long("output")
                .required(true)
                .takes_value(true)
                .help("Desired output path for the ExtraAction"),
        )
        .arg(
            Arg::with_name("crate_name")
                .long("crate_name")
                .required(true)
                .takes_value(true)
                .help("The name for the crate"),
        )
        .arg(
            Arg::with_name("owner")
                .long("owner")
                .required(true)
                .takes_value(true)
                .help("The action owner"),
        )
        .arg(
            Arg::with_name("sysroot")
                .long("sysroot")
                .required(true)
                .takes_value(true)
                .help("The Rust sysroot"),
        )
        .arg(
            Arg::with_name("linker")
                .long("linker")
                .required(true)
                .takes_value(true)
                .help("The linker path"),
        )
        .get_matches();

    let mut extra_action = ExtraActionInfo::new();
    extra_action.set_owner(matches.value_of("owner").unwrap().to_string());

    // Populate SpawnInfo
    let mut spawn_info = SpawnInfo::new();
    let source_files: Vec<String> =
        matches.value_of("src_files").unwrap().split(',').map(String::from).collect();
    let crate_name = matches.value_of("crate_name").unwrap();
    let main_source_file = if source_files.len() == 1 {
        &source_files[0]
    } else {
        &source_files[source_files
            .iter()
            .position(|file_path| file_path.contains(&"main.rs") || file_path.contains(&"lib.rs"))
            .unwrap()]
    };
    let arguments: Vec<String> = vec![
        "--".to_string(),
        // Only the main source file gets passed to the compiler
        main_source_file.to_string(),
        format!("-L{}", matches.value_of("sysroot").unwrap()),
        format!("--codegen=linker={}", matches.value_of("linker").unwrap()),
        format!("--crate-name={}", crate_name),
        // This path gets replaced by the extractor so it doesn't matter
        "--out-dir=/tmp".to_string(),
    ];
    spawn_info.set_argument(protobuf::RepeatedField::from_vec(arguments));
    spawn_info.set_input_file(protobuf::RepeatedField::from_vec(source_files));
    spawn_info.set_output_file(protobuf::RepeatedField::from_vec(vec![crate_name.to_string()]));

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
    let extra_action_path = Path::new(matches.value_of("output").unwrap());
    let mut extra_action_file = File::create(&extra_action_path)
        .with_context(|| "Failed to create ExtraActionInfo file".to_string())?;
    extra_action_file
        .write_all(&extra_action_info_bytes)
        .with_context(|| "Failed to write ExtraActionInfo to file".to_string())?;

    Ok(())
}
