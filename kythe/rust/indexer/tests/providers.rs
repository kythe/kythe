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
use kythe_rust_indexer::{error::KytheError, providers::{FileProvider, KzipFileProvider}};
use std::env;
use std::fs::File;
use std::path::{Path, PathBuf};

/// Get the path of the provided runfile
///
/// # Panics
///
/// Will panic if the TEST_SRCDIR environment variable isn't set.
fn get_runfile(file: &str) -> PathBuf {
  let testfiles_path = "io_kythe/kythe/rust/indexer/testfiles/";
  match env::var("TEST_SRCDIR") {
    Ok(path) => {
      let src_path = Path::new(&path);
      src_path.join(testfiles_path).join(file)
    }
    Err(_) => {
      panic!("TEST_SRCDIR environment variable not set or cannot be accessed.");
    }
  }
}

#[test]
fn test_kzip_provider() {
  // Load test kzip from the test files
  let kzip_path = get_runfile("testkzip.kzip");
  let kzip_file = match File::open(kzip_path) {
    Ok(file) => file,
    Err(_e) => {
      panic!("Test kzip file could not be found");
    }
  };

  // Create a new FileProvider using the test kzip
  let mut kzip_provider =
    match KzipFileProvider::new(kzip_file) {
      Ok(provider) => provider,
      Err(_e) => {
        panic!("Could not create a KzipFileProvider from the kzip");
      }
    };

  // Check the `exists` function
  let file_hash =
    "c9d04c9565fc665c80681fb1d829938026871f66e14f501e08531df66938a789";
  assert_eq!(
    kzip_provider.exists(file_hash),
    true,
    "File should exist in kzip but doesn't"
  );
  assert_eq!(
    kzip_provider.exists("invalid"),
    false,
    "File shouldn't exist but does"
  );

  // Check the `contents` function
  match kzip_provider.contents(file_hash) {
    Ok(contents) => {
      assert_eq!(contents, "Test\n", "File contents did not match expected contents");
    },
    Err(KytheError::FileNotFoundError) => {
      panic!("Got FileNotFoundError when the file exists");
    },
    Err(KytheError::FileReadError) => {
      panic!("Failed to read file to a string");
    },
    _ => {
      panic!("Got unexpected error when reading file contents");
    }
  };
  let invalid_file_contents = kzip_provider.contents("invalid");
  assert_eq!(invalid_file_contents.is_err(), true, "Expected Err but got Ok while reading invalid file contents");
  match invalid_file_contents.err().unwrap() {
    KytheError::FileNotFoundError => {},
    _ => {
      panic!("Expected FileNotFoundError but got other error");
    }
  };

  // Check `get_compilation_units` function
  match kzip_provider.get_compilation_units() {
    Ok(units) => {
      assert_eq!(units.len(), 0, "Expected units vector to be empty");
    },
    Err(_) => {
      panic!("Got error when getting compilation units from kzip");
    }
  }
}
