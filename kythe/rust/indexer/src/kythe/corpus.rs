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

use super::schema::{VName, Fact, NodeKind};
use super::writer::EntryWriter;
use std::fs::File;
use std::io::prelude::*;
use std::io;

// Corpora are used to generate nodes
// allowing them to control VName construction.
pub struct Corpus {
    pub name: String,
    pub writer: Box<EntryWriter>,
}

impl Corpus {
    // Creates a collection of entries describing a file node.
    // Can fail if file is unreachable.
    pub fn file_node(&self, path: &String) -> io::Result<()> {
        let vname = VName {
            path: Some(path.clone()),
            corpus: Some(self.name.clone()),
            ..Default::default()
        };
        let mut f = try!(File::open(path));
        let mut contents = String::new();
        try!(f.read_to_string(&mut contents));

        self.writer.node(&vname, Fact::Text, &contents);
        self.writer.node(&vname, Fact::NodeKind, &NodeKind::File);
        Ok(())
    }
}
