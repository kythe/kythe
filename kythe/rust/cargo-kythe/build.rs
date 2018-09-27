// Copyright 2016 The Kythe Authors. All rights reserved.
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

use std::env;
use std::fs;
use std::path::Path;
fn main() {
    let out_dir = env::var("OUT_DIR").expect("OUT_DIR variable must be set");
    let dep_dir = Path::new(&out_dir).join("../../../deps");
    let cargo_home = env::var("CARGO_HOME").expect("CARGO_HOME variable must be set");
    let so_folder = Path::new(&cargo_home).join("kythe");

    let indexer = fs::read_dir(dep_dir).expect("deps folder not found").find(|entry| {
        entry.as_ref().unwrap().file_name().to_string_lossy().starts_with("libkythe_indexer")
    });

    let indexer = indexer.expect("indexer not found").unwrap();
    fs::create_dir_all(so_folder.as_path()).expect("failed to create kythe directory");
    fs::copy(indexer.path(), so_folder.join(indexer.file_name()))
        .expect("failed to copy indexer to kythe directory");
}
