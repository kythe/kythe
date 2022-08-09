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
use clap::{App, Arg};
use serde_json::Value;
use std::io::{self, Write};

fn main() -> Result<()> {
    let mut file_provider = ProxyFileProvider::new();
    let mut kythe_writer = ProxyWriter::new();
    let mut indexer = KytheIndexer::new(&mut kythe_writer);

    let matches = App::new("Kythe Rust Proxy Indexer")
        .arg(
            Arg::with_name("no_emit_std_lib")
                .long("no_emit_std_lib")
                .required(false)
                .help("Disables emitting cross references to the standard library"),
        )
        .arg(
            Arg::with_name("tbuiltin_std_corpus")
                .long("tbuiltin_std_corpus")
                .required(false)
                .help("Emits built-in types in the \"std\" corpus"),
        )
        .get_matches();
    let emit_std_lib = !matches.is_present("no_emit_std_lib");
    let tbuiltin_std_corpus = matches.is_present("tbuiltin_std_corpus");

    // Request and process
    loop {
        let unit = request_compilation_unit()?;
        // Index the CompilationUnit and let the proxy know we are done
        let index_res =
            indexer.index_cu(&unit, &mut file_provider, emit_std_lib, tbuiltin_std_corpus);
        if index_res.is_ok() {
            send_done(true, String::new())?;
        } else {
            send_done(false, index_res.err().unwrap().to_string())?;
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
    if response["rsp"].as_str().unwrap() == "ok" {
        let args = response["args"].clone();
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
