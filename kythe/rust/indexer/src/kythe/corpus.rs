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

use super::schema::VName;

// Corpora are used to generate nodes
// allowing them to control VName construction.
pub struct Corpus {
    pub name: String,
}

// Constant for language field in VNames
static RUST: &'static str = "rust";

impl Corpus {
    // Generates the appropriate VName for a given file path.
    pub fn file_vname(&self, path: &str) -> VName {
        VName {
            path: Some(path.to_string()),
            corpus: Some(self.name.clone()),
            ..Default::default()
        }
    }

    // Generates the appropriate VName for an anchor in the text of a file.
    pub fn anchor_vname(&self, path: &str, start: usize, end: usize) -> VName {
        VName {
            path: Some(path.to_string()),
            corpus: Some(self.name.clone()),
            language: Some(RUST.to_string()),
            signature: Some(start.to_string() + "," + &end.to_string()),

            ..Default::default()
        }
    }

    // Generates the appropriate VName for a defintion
    // based on the name and definition id. This id is only
    // unique to the crate in which it resides.
    pub fn def_vname(&self, name: &str, def_id: u32) -> VName {
        VName {
            corpus: Some(self.name.clone()),
            language: Some(RUST.to_string()),
            signature: Some(format!("def:{}#{}", name, def_id)),
            ..Default::default()
        }
    }
}
