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

use kythe_rust_extractor::generate_analysis;
use std::env;
use std::fs::File;
use std::path::{Path, PathBuf};

use tempdir::TempDir;

#[test]
fn empty_args_fails() {
    let args: Vec<String> = Vec::new();
    let temp_dir = TempDir::new("extractor_test").expect("Could not create temporary directory");
    let analysis_directory = PathBuf::new().join(temp_dir.path());
    let result = generate_analysis(args, analysis_directory);
    assert_eq!(result.unwrap_err(), "Arguments vector should not be empty".to_string());
}

#[test]
fn nonempty_string_first_fails() {
    let args: Vec<String> = vec!["nonempty".to_string()];
    let temp_dir = TempDir::new("extractor_test").expect("Could not create temporary directory");
    let analysis_directory = PathBuf::new().join(temp_dir.path());
    let result = generate_analysis(args, analysis_directory);
    assert_eq!(result.unwrap_err(), "The first argument must be an empty string".to_string());
}

#[test]
fn correct_arguments_succeed() {
    // Get location of test file to be compiled
    let test_file_var = env::var_os("TEST_FILE").expect("TEST_FILE variable not set");
    let test_file =
        test_file_var.into_string().expect("Failed to convert TEST_FILE env variable to string");

    // Get sysroot location
    let sysroot_var = env::var_os("SYSROOT").expect("SYSROOT variable not set");
    let sysroot =
    sysroot_var.into_string().expect("Failed to convert SYSROOT env variable to string");

    // Generate the save_analysis
    let temp_dir = TempDir::new("extractor_test").expect("Could not create temporary directory");
    let args: Vec<String> = vec![
        "".to_string(),
        test_file.to_string(),
        format!("-L{}", sysroot),
        "--crate-name=test_crate".to_string(),
        format!("--out-dir={:?}", temp_dir.path()),
    ];
    let analysis_directory = PathBuf::new().join(temp_dir.path());
    let result = generate_analysis(args, analysis_directory);
    assert_eq!(result.unwrap(), (), "generate_analysis result wasn't void");

    // Ensure the save_analysis file exists
    let _ = File::open(Path::new(temp_dir.path()).join("save-analysis/test_crate.json"))
        .expect("save_analysis did not exist in the expected path");
}
