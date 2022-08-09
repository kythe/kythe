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
#[macro_use]
extern crate anyhow;

use analysis_rust_proto::*;
use anyhow::{Context, Result};
use extra_actions_base_rust_proto::*;
use protobuf::Message;
use runfiles::Runfiles;
use std::fs::File;
use std::io::{BufReader, Write};
use std::path::Path;
use std::process::Command;
use tempdir::TempDir;

fn main() -> Result<()> {
    // Setup temporary directory for the test and write the test source file
    let temp_dir = TempDir::new("rust_extractor_test")
        .with_context(|| "Failed to make temporary directory".to_string())?;
    let temp_dir_str = temp_dir
        .path()
        .to_str()
        .ok_or_else(|| anyhow!("Failed to convert temporary directory path to a string"))?;

    let mut extra_action = ExtraActionInfo::new();
    extra_action.set_owner("owner".to_string());

    let mut spawn_info = SpawnInfo::new();

    // Fill out SpawnInfo protobuf
    let rust_test_source = std::env::var("TEST_FILE").expect("TEST_FILE variable missing");
    let sysroot = std::env::var("SYSROOT").expect("SYSROOT variable missing");
    let arguments: Vec<String> = vec![
        "--".to_string(),
        "rustc".to_string(),
        rust_test_source.clone(),
        format!("-L{}", sysroot),
        "--crate-name=test_crate".to_string(),
        format!("--out-dir={}", temp_dir_str),
    ];
    spawn_info.set_argument(protobuf::RepeatedField::from_vec(arguments.clone()));

    let input_files: Vec<String> = vec![rust_test_source];
    spawn_info.set_input_file(protobuf::RepeatedField::from_vec(input_files));

    let output_key = format!("{}/test_crate", temp_dir_str);
    let output_files: Vec<String> = vec![output_key.clone()];
    spawn_info.set_output_file(protobuf::RepeatedField::from_vec(output_files));

    // If $CARGO_OUT_DIR is set, set the value as $OUT_DIR in the spawn info
    if let Ok(out_dir) = std::env::var("CARGO_OUT_DIR") {
        let mut env_var = EnvironmentVariable::new();
        env_var.set_name("OUT_DIR".to_string());
        env_var.set_value(out_dir);
        let vars = vec![env_var];
        spawn_info.set_variable(protobuf::RepeatedField::from_vec(vars));
    }

    // Write SpawnInfo bytes to a vector
    let spawn_info_bytes: Vec<u8> = spawn_info
        .write_to_bytes()
        .with_context(|| "Failed to convert SpawnInfo to bytes".to_string())?;

    // Add SpawnInfo extension to the ExtraActionInfo
    let action_unknown_fields = extra_action.mut_unknown_fields();
    // SpawnInfo is extension field 1003 on ExtraActionInfo
    // We have to use the `add_length_delimited` function but we only need to
    // pass the regulat bytes (not length delimited) of the SpawnInfo.
    action_unknown_fields.add_length_delimited(1003u32, spawn_info_bytes);

    // Write ExtraActionInfo to file
    let extra_action_info_bytes: Vec<u8> = extra_action
        .write_to_bytes()
        .with_context(|| "Failed to convert ExtraActionInfo to bytes".to_string())?;
    let extra_action_path = temp_dir.path().join("extra_action.xa");
    let extra_action_path_str = extra_action_path
        .to_str()
        .ok_or_else(|| anyhow!("Failed to convert extra action path to a string"))?;
    let mut extra_action_file = File::create(&extra_action_path)
        .with_context(|| "Failed to create ExtraActionInfo file".to_string())?;
    extra_action_file
        .write_all(&extra_action_info_bytes)
        .with_context(|| "Failed to write ExtraActionInfo to file".to_string())?;

    missing_arguments_fail();
    bad_extra_action_path_fails();
    correct_arguments_succeed(extra_action_path_str, temp_dir_str, &output_key, arguments);

    Ok(())
}

fn missing_arguments_fail() {
    let extractor_path = std::env::var("EXTRACTOR_PATH").expect("Couldn't find extractor path");
    let mut output = Command::new(&extractor_path).arg("--output=/tmp/wherever").output().unwrap();
    assert_eq!(output.status.code().unwrap(), 2);
    assert!(
        String::from_utf8_lossy(&output.stderr)
            .starts_with("error: The following required arguments were not provided:")
    );

    output = Command::new(&extractor_path).arg("--extra_action=/tmp/wherever").output().unwrap();
    assert_eq!(output.status.code().unwrap(), 2);
    assert!(
        String::from_utf8_lossy(&output.stderr)
            .starts_with("error: The following required arguments were not provided:")
    );

    output = Command::new(&extractor_path).output().unwrap();
    assert_eq!(output.status.code().unwrap(), 2);
    assert!(
        String::from_utf8_lossy(&output.stderr)
            .starts_with("error: The following required arguments were not provided:")
    );
}

fn bad_extra_action_path_fails() {
    let r = Runfiles::create().unwrap();
    let vnames_path = r.rlocation("io_kythe/external/io_kythe/kythe/data/vnames_config.json");
    let extractor_path = std::env::var("EXTRACTOR_PATH").expect("Couldn't find extractor path");

    let output = Command::new(&extractor_path)
        .arg("--extra_action=badPath")
        .arg("--output=/tmp/wherever")
        .arg(format!("--vnames_config={}", vnames_path.to_string_lossy()))
        .output()
        .unwrap();
    assert_eq!(output.status.code().unwrap(), 1);
    assert!(String::from_utf8_lossy(&output.stderr).contains("Failed to open extra action file"));
}

fn correct_arguments_succeed(
    extra_action_path_str: &str,
    temp_dir_str: &str,
    expected_output_key: &str,
    expected_arguments: Vec<String>,
) {
    let r = Runfiles::create().unwrap();
    let vnames_path = r.rlocation("io_kythe/external/io_kythe/kythe/data/vnames_config.json");

    let extractor_path = std::env::var("EXTRACTOR_PATH").expect("Couldn't find extractor path");
    let kzip_path_str = format!("{}/output.kzip", temp_dir_str);
    let exit_status = Command::new(&extractor_path)
        .arg(format!("--extra_action={}", extra_action_path_str))
        .arg(format!("--output={}", kzip_path_str))
        .arg(format!("--vnames_config={}", vnames_path.to_string_lossy()))
        .status()
        .unwrap();
    assert_eq!(exit_status.code().unwrap(), 0);

    // Open kzip
    let kzip_path = Path::new(&kzip_path_str);
    let kzip_file = File::open(kzip_path).expect("Couldn't open resulting kzip file");
    let mut kzip = zip::ZipArchive::new(kzip_file).expect("Couldn't read kzip archive");

    // Ensure the source file and OUT_DIR input from the test macro are in the kzip
    kzip.by_name("root/files/7cb3b3c74ecdf86f434548ba15c1651c92bf03b6690fd0dfc053ab09d094cf03")
        .expect("main.rs is missing from the generated kzip");
    kzip.by_name("root/files/75d66361585a3ca351b88047de1e1ddbbd3f85b070be2faf8f53f5e886447cf7")
        .expect("$OUT_DIR/input.rs is missing from the generated kzip");

    // Ensure protobuf is in the kzip
    let mut cu_path_str = String::from("");
    for i in 0..kzip.len() {
        let file = &kzip.by_index(i).expect("Couldn't get file in zip by index");
        if file.is_file() && file.name().contains("pbunits/") {
            cu_path_str = file.name().to_string();
        }
    }
    assert_ne!(cu_path_str, "", "IndexedCompilation protobuf missing from kzip");

    // Read it into an IndexedCompilation
    let cu_file = kzip.by_name(&cu_path_str).unwrap();
    let mut cu_reader = BufReader::new(cu_file);
    let indexed_compilation: IndexedCompilation =
        protobuf::Message::parse_from_reader(&mut cu_reader)
            .expect("Failed to parse protobuf as IndexedCompilation");

    // Ensure the CompilationUnit is correct
    let compilation_unit = indexed_compilation.get_unit();

    assert!(
        !compilation_unit.get_working_directory().is_empty(),
        "CompilationUnit working directory is not empty"
    );

    let source_files: Vec<String> = compilation_unit.get_source_file().to_vec();
    assert_eq!(source_files.len(), 2);
    assert!(
        source_files.get(0).unwrap().contains("kythe/rust/extractor/main.rs"),
        "The first source file's path doesn't contain \"kythe/rust/extractor/main.rs\": {}",
        source_files.get(0).unwrap()
    );
    assert!(
        source_files.get(1).unwrap().contains("out_dir/input.rs"),
        "The second source file's path doesn't contain \"out_dir/input.rs\": {}",
        source_files.get(1).unwrap()
    );

    let output_key = compilation_unit.get_output_key();
    assert_eq!(output_key, expected_output_key, "output_key field doesn't match");

    let unit_vname = compilation_unit.get_v_name();
    assert_eq!(unit_vname.get_language(), "rust", "VName language field doesn't match");
    assert_eq!(unit_vname.get_corpus(), "test_corpus", "VName corpus field doesn't match");

    let arguments = compilation_unit.get_argument();
    assert_eq!(arguments, expected_arguments, "Argument field doesn't match");

    let required_inputs = compilation_unit.get_required_input().to_vec();
    assert_eq!(
        required_inputs.len(),
        3,
        "Unexpected number of required inputs: {}",
        required_inputs.len()
    );

    // Test attributes of the required input for main.rs
    let source_input = required_inputs.get(0).expect("Failed to get the first required input");
    let source_file_path = source_input.get_info().get_path();
    assert!(
        source_file_path.contains("kythe/rust/extractor/main.rs"),
        "Path for the first required input doesn't contain \"kythe/rust/extractor/main.rs\": {}",
        source_file_path
    );
    assert_eq!(
        source_input.get_info().get_digest(),
        "7cb3b3c74ecdf86f434548ba15c1651c92bf03b6690fd0dfc053ab09d094cf03",
        "Incorrect digest for main.rs: {}",
        source_input.get_info().get_digest()
    );

    // Test attributes of the required input for $OUT_DIR/input.rs
    let out_dir_input = required_inputs.get(1).expect("Failed to get the second required input");
    let out_dir_input_path = out_dir_input.get_info().get_path();
    assert!(
        out_dir_input_path.contains("out_dir/input.rs"),
        "Path for the second required input doesn't contain \"out_dir/input.rs\": {}",
        out_dir_input_path
    );
    assert_eq!(
        out_dir_input.get_info().get_digest(),
        "75d66361585a3ca351b88047de1e1ddbbd3f85b070be2faf8f53f5e886447cf7",
        "Incorrect digest for $OUT_DIR/input.rs: {}",
        out_dir_input.get_info().get_digest()
    );

    // Test attributes of the required_input for the save analysis
    let analysis_input = required_inputs.get(2).expect("Failed to get the third required input");
    let analysis_path = analysis_input.get_info().get_path();
    assert!(
        analysis_path.contains("save-analysis/test_crate.json"),
        "Unexpected file path for save_analysis file: {}",
        analysis_path
    );
}
