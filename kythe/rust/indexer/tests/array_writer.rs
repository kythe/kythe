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

use kythe_rust_indexer::{error::KytheError, writer::KytheWriter};
use storage_rust_proto::*;

/// A [KytheWriter] that writes entries to a Vec.
///
/// Used for testing the indexer without needing to decode a CodedOutputStream
#[derive(Default)]
pub struct ArrayWriter {
    entries: Vec<Entry>,
}

impl ArrayWriter {
    /// Returns a new ArrayWriter
    pub fn new() -> Self {
        Self { entries: Vec::new() }
    }
}

impl KytheWriter for ArrayWriter {
    fn write_entry(&mut self, entry: Entry) -> Result<(), KytheError> {
        self.entries.push(entry);
        Ok(())
    }

    fn flush(&mut self) -> Result<(), KytheError> {
        Ok(())
    }
}

impl AsRef<Vec<Entry>> for ArrayWriter {
    fn as_ref(&self) -> &Vec<Entry> {
        self.entries.as_ref()
    }
}

#[test]
fn array_writer_test() {
    let entry = Entry::new();
    let mut test_vec = Vec::new();
    test_vec.push(entry.clone());

    let mut writer = ArrayWriter::new();
    assert_eq!(writer.write_entry(entry).unwrap(), ());
    assert_eq!(writer.as_ref().clone(), test_vec);
}
