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
use clap::Parser;
use std::fs::File;
use std::path::PathBuf;

#[derive(Parser)]
#[clap(author = "The Kythe Authors")]
#[clap(about = "Kythe Rust Bazel Indexer", long_about = None)]
#[clap(rename_all = "snake_case")]
struct Args {
    /// The path to the kzip to be indexed
    #[clap(value_parser, value_name = "FILE")]
    kzip_path: PathBuf,

    /// Disables emitting cross references to the standard library
    #[clap(long, default_value_t = false)]
    no_emit_std_lib: bool,

    /// Emits built-in types in the "std" corpus
    #[clap(long, default_value_t = false)]
    tbuiltin_std_corpus: bool,
}

fn main() -> Result<()> {
    let args = Args::parse();
    let emit_std_lib = !args.no_emit_std_lib;

    // Get kzip path from argument and use it to create a KzipFileProvider
    // Unwrap is safe because the parameter is required
    let kzip_file = File::open(&args.kzip_path).context("Provided path does not exist")?;
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
        indexer.index_cu(&unit, &mut kzip_provider, emit_std_lib, args.tbuiltin_std_corpus)?;
    }
    Ok(())
}
