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

extern crate kythe_rust_indexer;
use kythe_rust_indexer::{
    error::KytheError,
    providers::{FileProvider, KzipFileProvider},
};
extern crate runfiles;
use runfiles::Runfiles;
use std::fs::File;
use std::path::PathBuf;

/// Get the path of the provided runfile
fn get_runfile(file: &str) -> PathBuf {
    let r = Runfiles::create().unwrap();
    r.rlocation("io_kythe/kythe/rust/indexer/tests/".to_owned() + file)
}

#[test]
fn test_kzip_provider() {
    // Load test kzip from the test files
    let kzip_path = get_runfile("testkzip.kzip");
    let kzip_file = File::open(kzip_path).expect("Test kzip file could not be found");

    // Create a new FileProvider using the test kzip
    let mut kzip_provider = KzipFileProvider::new(kzip_file).unwrap();

    // Check the `exists` function
    let file_hash = "c9d04c9565fc665c80681fb1d829938026871f66e14f501e08531df66938a789";
    assert!(
        kzip_provider.exists("/tmp/main.rs", file_hash).unwrap(),
        "File should exist in kzip but doesn't"
    );
    assert!(!kzip_provider.exists("invalid", "invalid").unwrap(), "File shouldn't exist but does");

    // Check the `contents` function
    let contents_result = kzip_provider.contents("/tmp/main.rs", file_hash);
    assert!(!contents_result.is_err());
    let contents_string =
        String::from_utf8(contents_result.unwrap()).expect("File contents was not valid UTF-8");
    assert_eq!(contents_string, "Test\n", "File contents did not match expected contents");

    let invalid_contents = kzip_provider.contents("invalid", "invalid");
    assert!(invalid_contents.is_err(), "Expected Err while reading contents for non-existent file, but received file contents: {:?}", invalid_contents.unwrap());
    let contents_error = invalid_contents.err().unwrap();
    match contents_error {
        KytheError::FileNotFoundError(_) => {}
        _ => panic!(
            "Unexpected error while getting contents of nonexistent file: {}",
            contents_error
        ),
    }

    // Check `get_compilation_units` function
    let units = kzip_provider.get_compilation_units().unwrap();
    assert_eq!(units.len(), 0, "Expected units vector to be empty");
}
