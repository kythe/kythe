// Copyright 2021 The Kythe Authors. All rights reserved.
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
use kythe_rust_indexer::{indexer::KytheIndexer, providers::*, writer::ProxyWriter};

use analysis_rust_proto::*;
use anyhow::{anyhow, Context, Result};
use serde_json::Value;
use std::{
    fs::File,
    io::{self, Write},
    path::{Path, PathBuf},
};
use tempdir::TempDir;

fn main() -> Result<()> {
    let mut file_provider = ProxyFileProvider::new();
    let mut kythe_writer = ProxyWriter::new();
    let mut indexer = KytheIndexer::new(&mut kythe_writer);

    // Request and process
    loop {
        let unit = request_compilation_unit()?;
        // Retrieve the save_analysis file
        let temp_dir = TempDir::new("rust_indexer").context("Couldn't create temporary directory")?;
        let temp_path = PathBuf::new().join(temp_dir.path());
        if let Err(err) = write_analysis_to_directory(&unit, &temp_path, &mut file_provider) {
            println!(r#"{{"req":"done", "args"{{"ok":"false","msg":"{:?}"}}}}"#, err);
            let mut response_string = String::new();
            io::stdin()
                .read_line(&mut response_string)
                .context("Failed to read response from proxy")?;
            continue;
        }

        // Index the CompilationUnit and let the proxy know we are done
        match indexer.index_cu(&unit, &temp_path, &mut file_provider) {
            Ok(_) => {
                println!(r#"{{"req":"done", "args"{{"ok":"true","msg":""}}}}"#);
            },
            Err(err) => {
                println!(r#"{{"req":"done", "args"{{"ok":"false","msg":"{:?}"}}}}"#, err);
            },
        };

        // Grab the response, but we don't care what it is so just throw it away
        let mut response_string = String::new();
        io::stdin()
            .read_line(&mut response_string)
            .context("Failed to read response from proxy")?;
    }
}

/// Takes analysis files present in a CompilationUnit, requests the files
/// from the proxy, and writes them to `temp_path`
pub fn write_analysis_to_directory(
    c_unit: &CompilationUnit,
    temp_path: &Path,
    provider: &mut dyn FileProvider,
) -> Result<()> {
    for required_input in c_unit.get_required_input() {
        let input_path = required_input.get_info().get_path();
        let input_path_buf = PathBuf::new().join(input_path);

        // save_analysis files are JSON files
        if let Some(os_str) = input_path_buf.extension() {
            if let Some("json") = os_str.to_str() {
                let digest = required_input.get_info().get_digest();
                let file_contents = provider.contents(&input_path, digest).with_context(|| {
                    format!(
                        "Failed to get contents of file \"{}\" with digest \"{}\"",
                        input_path, digest
                    )
                })?;

                let output_path = temp_path.join(input_path_buf.file_name().unwrap());
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
        }
    }
    Ok(())
}

fn request_compilation_unit() -> Result<CompilationUnit> {
    println!(r#"{{"req":"analysis_wire"}}"#);
    io::stdout().flush().context("Failed to flush stdout")?;

    // Read the response from the proxy
    let mut response_string = String::new();
    io::stdin()
        .read_line(&mut response_string)
        .context("Failed to read response from proxy")?;

    // Convert to json and extract information
    let response: Value = serde_json::from_str(&response_string)
        .context("Failed to parse response as JSON")?;
    if response["rsp"].as_str().unwrap() == "ok" {
        let args = response["args"].clone();
        let unit_base64 = args["unit"].as_str().unwrap();
        let unit_bytes = hex::decode(unit_base64).context("Failed to decode CompilationUnit sent by proxy")?;
        let unit = protobuf::parse_from_bytes::<CompilationUnit>(&unit_bytes)
            .context("Failed to parse protobuf")?;
        Ok(unit)
    } else {
        Err(anyhow!("Failed to get a CompilationUnit from the proxy"))
    }
}
