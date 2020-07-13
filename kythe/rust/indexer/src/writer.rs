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

use crate::error::KytheError;
use protobuf::{self, CodedOutputStream, Message};
use std::io::Write;
use storage_rust_proto::*;

/// A trait for writing Kythe graph entries to an output.
pub trait KytheWriter {
    /// Write an entry to the output.
    ///
    /// Given an Kythe storage proto Entry, write that entry to the output of
    /// the KytheWriter. If the operation is successful, the function will
    /// return Ok with a void value.KytheError
    ///
    /// # Errors
    ///
    /// If the writer fails to write the entry to output, a
    /// [WriterError][KytheError::WriterError] will be returned.
    fn write_entry(&mut self, entry: Entry) -> Result<(), KytheError>;

    /// Flushes the CodedOutputStream buffer to output
    ///
    /// # Errors
    ///
    /// If an error occurs while flushing, a [KytheError::WriterError] will be
    /// returned.
    fn flush(&mut self) -> Result<(), KytheError>;
}

/// A [KytheWriter] that writes entries to a [CodedOutputStream]
pub struct StreamWriter<'a> {
    output_stream: CodedOutputStream<'a>,
}

impl<'a> StreamWriter<'a> {
    /// Create a new instance of StreamWriter
    ///
    /// Given a writer that implements the `Write` trait, initializes a
    /// CodedOutputStream and returns a new [StreamWriter](crate::StreamWriter).
    pub fn new(writer: &'a mut dyn Write) -> StreamWriter<'a> {
        Self { output_stream: CodedOutputStream::new(writer) }
    }
}

impl<'a> KytheWriter for StreamWriter<'a> {
    /// Given an [Entry], writes the entry using a [CodedOutputStream].
    /// First writes a varint32 of the size of the entry, then writes the actual
    /// entry.
    ///
    /// # Errors
    ///
    /// An error is returned if the write fails.
    fn write_entry(&mut self, entry: Entry) -> Result<(), KytheError> {
        let entry_size = entry.compute_size();
        self.output_stream.write_raw_varint32(entry_size).map_err(KytheError::WriterError)?;
        entry.write_to_with_cached_sizes(&mut self.output_stream).map_err(KytheError::WriterError)
    }

    fn flush(&mut self) -> Result<(), KytheError> {
        self.output_stream.flush().map_err(KytheError::WriterError)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn stream_writer_test() {
        // Create new test entry
        let mut entry = Entry::new();
        entry.set_edge_kind("aliases".into());

        // Write to a vec of bytes
        let mut bytes: Vec<u8> = Vec::new();
        let mut writer = StreamWriter::new(&mut bytes);
        assert!(writer.write_entry(entry.clone()).is_ok());
        assert!(writer.flush().is_ok());
        assert_eq!(bytes.len(), 10);

        // Size entry should be 9
        assert_eq!(bytes.get(0).unwrap(), &9u8);

        // Parse the other bytes into an Entry and check equality with original
        let entry_bytes = bytes.get(1..bytes.len()).unwrap();
        let parsed_entry = protobuf::parse_from_bytes::<Entry>(entry_bytes).unwrap();
        assert_eq!(entry, parsed_entry);
    }
}
