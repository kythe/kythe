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
use kythe_rust_indexer::{indexer::KytheIndexer, providers::*, proxyrequests, writer::ProxyWriter};

use analysis_rust_proto::*;
use anyhow::{anyhow, Context, Result};
use clap::Parser;
use serde_json::Value;
use std::io::{self, Write};

#[derive(Parser)]
#[clap(author = "The Kythe Authors")]
#[clap(about = "Kythe Rust Proxy Indexer", long_about = None)]
#[clap(rename_all = "snake_case")]
struct Args {
    /// Disables emitting cross references to the standard library
    #[clap(long, action)]
    no_emit_std_lib: bool,

    /// Emits built-in types in the "std" corpus
    #[clap(long, action)]
    tbuiltin_std_corpus: bool,
}

fn main() -> Result<()> {
    let mut file_provider = ProxyFileProvider::new();
    let mut kythe_writer = ProxyWriter::default();
    let mut indexer = KytheIndexer::new(&mut kythe_writer);

    let args = Args::parse();
    let emit_std_lib = !args.no_emit_std_lib;

    // Request and process
    loop {
        let unit = request_compilation_unit()?;
        // Index the CompilationUnit and let the proxy know we are done
        match indexer.index_cu(&unit, &mut file_provider, emit_std_lib, args.tbuiltin_std_corpus) {
            Ok(_) => send_done(true, String::new())?,
            Err(e) => send_done(false, e.to_string())?,
        }
    }
}

fn request_compilation_unit() -> Result<CompilationUnit> {
    println!(r#"{{"req":"analysis_wire"}}"#);
    io::stdout().flush().context("Failed to flush stdout")?;

    // Read the response from the proxy
    let mut response_string = String::new();
    io::stdin().read_line(&mut response_string).context("Failed to read response from proxy")?;

    // Convert to json and extract information
    let response: Value =
        serde_json::from_str(&response_string).context("Failed to parse response as JSON")?;
    if response["rsp"].as_str().unwrap_or_default() == "ok" {
        let args = &response["args"];
        let unit_base64 = args["unit"].as_str().unwrap();
        let unit_bytes = base64::decode(unit_base64)
            .context("Failed to decode CompilationUnit sent by proxy")?;
        let unit: CompilationUnit =
            protobuf::Message::parse_from_bytes(&unit_bytes).context("Failed to parse protobuf")?;
        Ok(unit)
    } else {
        Err(anyhow!("Failed to get a CompilationUnit from the proxy"))
    }
}

/// Sends the "done" request to the proxy. The first argument sets the "ok"
/// value, and the second sets the error message if the first argument is false.
fn send_done(ok: bool, msg: String) -> Result<()> {
    let request = proxyrequests::done(ok, msg)?;
    println!("{}", request);
    io::stdout().flush().context("Failed to flush stdout")?;

    // Grab the response, but we don't care what it is so just throw it away
    let mut response_string = String::new();
    io::stdin().read_line(&mut response_string).context("Failed to read response from proxy")?;
    Ok(())
}
