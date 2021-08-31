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

use kythe_rust_extractor::{generate_analysis, vname_util};
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
    let test_file = env::var("TEST_FILE").expect("TEST_FILE variable not set");

    // Get sysroot location
    let sysroot = env::var("SYSROOT").expect("SYSROOT variable not set");

    // Generate the save_analysis
    let temp_dir = TempDir::new("extractor_test").expect("Could not create temporary directory");
    let args: Vec<String> = vec![
        "".to_string(),
        test_file.to_string(),
        format!("-L{}", sysroot),
        "--crate-name=test_crate".to_string(),
        format!("--out-dir={}", temp_dir.path().to_str().unwrap()),
    ];
    let analysis_directory = PathBuf::new().join(temp_dir.path());
    let result = generate_analysis(args, analysis_directory);
    assert_eq!(result.unwrap(), (), "generate_analysis result wasn't void");

    // Ensure the save_analysis file exists
    let _ = File::open(Path::new(temp_dir.path()).join("save-analysis/test_crate.json"))
        .expect("save_analysis did not exist in the expected path");
}

#[test]
fn test_raw_rule_to_vname_rule() {
    let raw_rule = vname_util::RawRule {
        pattern: r"(Test\d+)+".to_string(),
        vname: vname_util::RawRulePatterns {
            corpus: Some("corpus".to_string()),
            root: Some("@1@@2@".to_string()),
            path: None,
        },
    };
    let vname_rule = raw_rule.process();
    assert_eq!(vname_rule.corpus_pattern, Some("corpus".to_string()));
    assert_eq!(vname_rule.root_pattern, Some("${1}${2}".to_string()));
    assert_eq!(vname_rule.path_pattern, None);
}

#[test]
fn test_vname_translation() {
    let mut vname_rule_1 = vname_util::VNameRule {
        pattern: regex::Regex::new("external/([^/]+)/(.*\\.rs)$")
            .expect("Couldn't parse test regex"),
        corpus_pattern: Some("rust_extractor".to_string()),
        root_pattern: Some("${1}".to_string()),
        path_pattern: Some("${2}".to_string()),
    };
    let path = "external/rust_test/vname_rules/lib.rs";
    assert!(vname_rule_1.matches(path));

    let vname_1 = vname_rule_1.produce_vname(path, "");
    assert_eq!(vname_1.get_corpus(), "rust_extractor".to_string());
    assert_eq!(vname_1.get_root(), "rust_test".to_string());
    assert_eq!(vname_1.get_path(), "vname_rules/lib.rs".to_string());

    let mut vname_rule_2 = vname_util::VNameRule {
        pattern: regex::Regex::new("external/([^/]+)/(.*\\.rs)$")
            .expect("Couldn't parse test regex"),
        corpus_pattern: None,
        root_pattern: Some("${1}".to_string()),
        path_pattern: Some("${2}".to_string()),
    };
    let path = "external/rust_test/vname_rules/lib.rs";
    assert!(vname_rule_2.matches(path));

    let vname_2 = vname_rule_2.produce_vname(path, "default");
    assert_eq!(vname_2.get_corpus(), "default".to_string());
    assert_eq!(vname_2.get_root(), "rust_test".to_string());
    assert_eq!(vname_2.get_path(), "vname_rules/lib.rs".to_string());
}
