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
use serde_json::Value;
use std::io::{self, Write};
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
pub struct CodedOutputStreamWriter<'a> {
    output_stream: CodedOutputStream<'a>,
}

impl<'a> CodedOutputStreamWriter<'a> {
    /// Create a new instance of CodedOutputStreamWriter
    ///
    /// Takes a writer that implements the `Write` trait as the only argument.
    pub fn new(writer: &'a mut dyn Write) -> CodedOutputStreamWriter<'a> {
        Self {
            output_stream: CodedOutputStream::new(writer),
        }
    }
}

impl<'a> KytheWriter for CodedOutputStreamWriter<'a> {
    /// Given an [Entry], writes the entry using a [CodedOutputStream].
    /// First writes a varint32 of the size of the entry, then writes the actual
    /// entry.
    ///
    /// # Errors
    ///
    /// An error is returned if the write fails.
    fn write_entry(&mut self, entry: Entry) -> Result<(), KytheError> {
        let entry_size_bytes = entry.compute_size();
        self.output_stream
            .write_raw_varint32(entry_size_bytes)
            .map_err(KytheError::WriterError)?;
        entry
            .write_to_with_cached_sizes(&mut self.output_stream)
            .map_err(KytheError::WriterError)
    }

    fn flush(&mut self) -> Result<(), KytheError> {
        self.output_stream.flush().map_err(KytheError::WriterError)
    }
}

/// A [KytheWriter] that communicates with the analysis driver proxy
pub struct ProxyWriter {
    buffer: Vec<String>,
}

impl ProxyWriter {
    /// Create a new instance of ProxyWriter
    ///
    /// Takes a writer that implements the `Write` trait as the only argument.
    pub fn new() -> ProxyWriter {
        Self {
            buffer: Vec::new(),
        }
    }
}

impl Default for ProxyWriter {
    fn default() -> Self {
        Self::new()
    }
}

impl KytheWriter for ProxyWriter {
    /// Given an [Entry], writes the entry to the buffer.
    /// If the buffer reaches size 1000, automatically flush to the proxy.
    ///
    /// # Errors
    ///
    /// An error is returned if the write fails.
    fn write_entry(&mut self, entry: Entry) -> Result<(), KytheError> {
        let bytes = entry.write_to_bytes().map_err(KytheError::WriterError)?;
        // Convert the entry bytes to base64 and surround with quotes to prepare for inserting into a JSON
        // array
        let b64 = format!(r#""{}""#, hex::encode(&bytes));
        self.buffer.push(b64);
        if self.buffer.len() >= 1000 {
            self.flush()?;
        }
        Ok(())
    }

    fn flush(&mut self) -> Result<(), KytheError> {
        // Combine all of the entries
        let buffer_json_array = self.buffer.join(", ");
        self.buffer.clear();

        // Write the proxy command to output
        println!(r#"{{"req":"output_wire","args":[{}]}}"#, buffer_json_array);
        io::stdout().flush()
            .map_err(|err| KytheError::IndexerError(format!("Failed to flush stdout: {:?}", err)))?;

        // Read the response from the proxy
        let mut response_string = String::new();
        io::stdin()
            .read_line(&mut response_string)
            .map_err(|err| KytheError::IndexerError(format!("Failed to read proxy file response from stdin: {:?}", err)))?;

        // Convert to json and extract information
        let response: Value = serde_json::from_str(&response_string)
            .map_err(|err| KytheError::IndexerError(format!("Failed to read proxy file response from stdin: {:?}", err)))?;
        if response["rsp"].as_str().unwrap() == "error" {
            Err(KytheError::IndexerError(
                format!("Got error back from proxy after sending entries: {}", response["args"].as_str().unwrap())
            ))
        } else {
            Ok(())
        }
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
        let mut writer = CodedOutputStreamWriter::new(&mut bytes);
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
