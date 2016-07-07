// Copyright 2016 Google Inc. All rights reserved.
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

use rustc_serialize::base64::{ToBase64, STANDARD};
use rustc_serialize::json;
use super::schema::{VName, Fact, EdgeKind};

// EntryWriter implementations define behavior for outputting kythe info.
pub trait EntryWriter {
    fn edge(&self, src: &VName, edge_kind: EdgeKind, target: &VName);
    // TODO(djrenren): Make fact_value &[u8] instead of &str
    fn node(&self, src: &VName, fact_name: Fact, fact_value: &str);
}

// Struct used for writing kythe info to STDOUT in json form
pub struct JsonEntryWriter;

#[derive(RustcEncodable)]
struct Node<'a> {
    source: &'a VName,
    fact_name: &'a str,
    fact_value: String,
}

#[derive(RustcEncodable)]
struct Edge<'a> {
    source: &'a VName,
    edge_kind: &'a str,
    target: &'a VName,
    fact_name: &'a str,
}


impl EntryWriter for JsonEntryWriter {
    fn edge(&self, src: &VName, edge_kind: EdgeKind, target: &VName) {
        let edge = Edge {
            source: src,
            edge_kind: &edge_kind,
            target: target,
            fact_name: "/",
        };

        let json_str = json::encode(&edge).unwrap();
        println!("{}", json_str);
    }

    fn node(&self, src: &VName, fact_name: Fact, fact_value: &str) {
        let node = Node {
            source: src,
            fact_name: &fact_name,
            fact_value: fact_value.as_bytes().to_base64(STANDARD),
        };

        let json_str = json::encode(&node).unwrap();
        println!("{}", json_str);
    }
}
