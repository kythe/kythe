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

extern crate kythe_rust_indexer;
use kythe_rust_indexer::{indexer::KytheIndexer, providers::*, writer::CodedOutputStreamWriter};

use analysis_rust_proto::*;
use anyhow::{Context, Result};
use std::{
    env,
    fs::File,
    io::Write,
    path::{Path, PathBuf},
};
use tempdir::TempDir;

fn main() -> Result<()> {
    // Accepts kzip path as an argument
    // Calls indexer on each compilation unit
    // Returns 0 if ok or 1 if error
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Not enough arguments! Usage: rust_indexer <kzip path>");
        std::process::exit(1);
    } else if args.len() > 3 {
        eprintln!("Too many arguments! Usage: rust_indexer <kzip path>")
    }

    // Get kzip path from argument and use it to create a KzipFileProvider
    let kzip_path = Path::new(&args[1]);
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
        // Create a temporary directory to store required files
        let temp_dir =
            TempDir::new("rust_indexer").context("Couldn't create temporary directory")?;
        let temp_path = PathBuf::new().join(temp_dir.path());

        // Extract all of the files from the kzip into the temporary directory
        extract_from_kzip(&unit, &temp_path, &mut kzip_provider)?;

        // Index the CompilationUnit
        indexer.index_cu(&unit, &temp_path)?;
    }
    Ok(())
}

/// Takes files from a kzip loaded into `provider` and extracts them to
/// `temp_path` using the file names and digests in the CompilationUnit
pub fn extract_from_kzip(
    c_unit: &CompilationUnit,
    temp_path: &Path,
    provider: &mut dyn FileProvider,
) -> Result<()> {
    for required_input in c_unit.get_required_input() {
        // Get path where we will copy the file to
        let input_path = required_input.get_info().get_path();

        // The only JSON files in the kzip are the save_analyses, so we put them in the
        // analysis folder.
        let input_path_buf = PathBuf::new().join(input_path);
        let output_path = match input_path_buf.extension() {
            Some(os_str) => match os_str.to_str() {
                Some("json") => {
                    temp_path.join("analysis").join(input_path_buf.file_name().unwrap())
                }
                _ => temp_path.join(input_path),
            },
            _ => temp_path.join(input_path),
        };

        // Create directories for the output path
        let parent = output_path
            .parent()
            .ok_or_else(|| anyhow!("Failed to get parent for path: {:?}", output_path))?;
        std::fs::create_dir_all(parent).with_context(|| {
            format!("Failed to create temporary directories for path: {}", parent.display())
        })?;

        // Copy the file contents to the output path in the temporary directory
        let digest = required_input.get_info().get_digest();
        let file_contents = provider.contents(digest).with_context(|| {
            format!("Failed to get contents of file \"{}\" with digest \"{}\"", input_path, digest)
        })?;
        let mut output_file = File::create(&output_path).context("Failed to create file")?;
        output_file.write_all(&file_contents).with_context(|| {
            format!(
                "Failed to copy contents of \"{}\" with digest \"{}\" to \"{}\"",
                input_path,
                digest,
                output_path.display()
            )
        })?;
    }
    Ok(())
}
