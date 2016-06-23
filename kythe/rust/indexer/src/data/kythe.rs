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
use rustc_serialize::{json, Encodable, Encoder};

#[derive(RustcEncodable)]
pub struct Node {
    source: VName,
    fact_name: &'static str,
    fact_value: String,
}

#[derive(RustcEncodable)]
pub struct Edge {
    source: VName,
    edge_kind: &'static str,
    target: VName,
}

#[allow(dead_code)] // No edges being reported yet.
pub enum Entry {
    Node(Node),
    Edge(Edge),
}

impl Entry {
    #[allow(dead_code)] // No edges being reported yet.
    pub fn edge(src: VName, edge_kind: &'static str, target: VName) -> Entry {
        Entry::Edge(Edge {
            source: src,
            edge_kind: edge_kind,
            target: target,
        })
    }

    pub fn node(src: VName, fact_name: &'static str, fact_value: &str) -> Entry {
        Entry::Node(Node {
            source: src,
            fact_name: fact_name,
            fact_value: fact_value.as_bytes().to_base64(STANDARD),
        })
    }

    pub fn encode(&self) -> String {
        json::encode(self).unwrap()
    }
}

// Keep the JSON output free from enum variant info
// that the derived impl would include.
impl Encodable for Entry {
    fn encode<S: Encoder>(&self, s: &mut S) -> Result<(), S::Error> {
        match self {
            &Entry::Node(ref n) => n.encode(s),
            &Entry::Edge(ref e) => e.encode(s),
        }
    }
}

#[derive(Default, Clone, Debug, RustcEncodable)]
pub struct VName {
    pub signature: Option<String>,
    pub corpus: Option<String>,
    pub root: Option<String>,
    pub path: Option<String>,
    pub language: Option<String>,
}

macro_rules! pub_str {
    ($id:ident = $val:expr) => (pub static $id: &'static str = $val;)
}

pub mod node_kind {
    pub_str!(FILE = "file");
}

pub mod fact {
    pub_str!(NODE_KIND = "/kythe/node/kind");
    pub_str!(TEXT = "/kythe/text");
}
