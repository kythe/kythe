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
use kythe_rust_indexer::{indexer::KytheIndexer, providers::*, writer::CodedOutputStreamWriter};

use anyhow::{Context, Result};
use clap::{App, Arg};
use std::fs::File;

fn main() -> Result<()> {
    // Returns 0 if ok or 1 if error
    let matches = App::new("Kythe Rust Bazel Indexer")
        .arg(
            Arg::with_name("kzip_path")
                .required(true)
                .index(1)
                .help("The path to the kzip to be indexed"),
        )
        .arg(
            Arg::with_name("no_emit_std_lib")
                .long("no_emit_std_lib")
                .required(false)
                .help("Disables emitting cross references to the standard library"),
        )
        .get_matches();
    let emit_std_lib = !matches.is_present("no_emit_std_lib");

    // Get kzip path from argument and use it to create a KzipFileProvider
    // Unwrap is safe because the parameter is required
    let kzip_path = matches.value_of("kzip_path").unwrap();
    let kzip_file = File::open(kzip_path).context("Provided path does not exist")?;
    let mut kzip_provider =
        KzipFileProvider::new(kzip_file).context("Failed to open kzip archive")?;
    let compilation_units = kzip_provider
        .get_compilation_units()
        .context("Failed to get compilation units from kzip")?;

    // Create instances of StreamWriter and KytheIndexer
    let mut stdout_writer = std::io::stdout();
    let mut writer = CodedOutputStreamWriter::new(&mut stdout_writer);
    let mut indexer = KytheIndexer::new(&mut writer);

    for unit in compilation_units {
        indexer.index_cu(&unit, &mut kzip_provider, emit_std_lib)?;
    }
    Ok(())
}
