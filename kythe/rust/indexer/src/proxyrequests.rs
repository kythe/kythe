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

use serde::Serialize;
use serde_json::Result;

#[derive(Serialize, Debug)]
struct ProxyFileRequestArgs {
    path: String,
    digest: String,
}

#[derive(Serialize, Debug)]
struct ProxyFileRequest {
    req: String,
    args: ProxyFileRequestArgs,
}

#[derive(Serialize, Debug)]
struct ProxyDoneRequestArgs {
    ok: bool,
    msg: String,
}

#[derive(Serialize, Debug)]
struct ProxyDoneRequest {
    req: String,
    args: ProxyDoneRequestArgs,
}

#[derive(Serialize, Debug)]
struct ProxyOutputRequest {
    req: String,
    args: Vec<String>,
}

/// Create a proxystub command for requesting file information
pub fn file(path: String, digest: String) -> Result<String> {
    let request =
        ProxyFileRequest { req: String::from("file"), args: ProxyFileRequestArgs { path, digest } };
    serde_json::to_string(&request)
}

/// Create a proxystub command indicating we are done indexing a compilation
/// unit
pub fn done(ok: bool, msg: String) -> Result<String> {
    let request =
        ProxyDoneRequest { req: String::from("done"), args: ProxyDoneRequestArgs { ok, msg } };
    serde_json::to_string(&request)
}

/// Create a proxystub command outputting entries
pub fn output(args: Vec<String>) -> Result<String> {
    let request = ProxyOutputRequest { req: String::from("output_wire"), args };
    serde_json::to_string(&request)
}
