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
use kythe_rust_indexer::indexer::save_analysis::load_analysis;

extern crate runfiles;
use runfiles::Runfiles;

extern crate tempdir;
use tempdir::TempDir;

use std::fs;
use std::path::PathBuf;

#[test]
fn load_analysis_works_properly() {
    let temp_dir = TempDir::new("analysis_test").expect("Couldn't create temporary directory");
    let temp_path = PathBuf::new().join(temp_dir.path());

    let r = Runfiles::create().unwrap();
    let path =  r.rlocation("io_kythe/kythe/rust/indexer/tests/testanalysis.json");
    fs::copy(&path, temp_path.join("main.json")).expect("Couldn't copy file");

    let crates = load_analysis(&temp_path);
    temp_dir.close().expect("Couldn't clean up temporary directory");

    assert_eq!(crates.len(), 1);
}
