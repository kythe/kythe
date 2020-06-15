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
use analysis_rust_proto::*;
use rc_zip::{prelude::*, Archive, EntryContents};
use std::collections::HashMap;
use std::fs::File;
use std::io::Read;

/// A trait for retrieving files during indexing.
pub trait FileProvider {
    /// Checks whether a file exists and that the contents are available.
    ///
    /// This function takes a file hash and returns whether
    /// the file exists and that the contents can be requested.
    ///
    /// # Errors
    ///
    /// If the file is not a valid zip archive, a [KytheError::KzipFileError]
    /// will be returned.
    fn exists(&mut self, file_hash: &str) -> bool;

    /// Retrieves the string contents of a file.
    ///
    /// This function takes a file hash and returns the string contents of the
    /// file.
    ///
    /// # Errors
    ///
    /// If the file does not exist, a
    /// [FileNotFoundError][KytheError::FileNotFoundError] will be returned.
    /// If the file can't be read into a String, a
    /// [FileReadError][KytheError::FileReadError] will be returned.
    fn contents(&mut self, file_hash: &str) -> Result<String, KytheError>;
}

/// A [FileProvider] that backed by a .kzip file.
pub struct KzipFileProvider {
    file: File,
    file_map: HashMap<String, usize>,
    zip_archive: Archive,
}

impl KzipFileProvider {
    /// Create a new instance using a valid [File] object of a kzip
    /// file.
    ///
    /// # Errors
    ///
    /// If the file is not a valid zip archive, a
    /// [KzipFileError][KytheError::KzipFileError] will be returned.
    pub fn new(file: File) -> Result<Self, KytheError> {
        let zip_archive = file.read_zip()?;
        let mut file_map = HashMap::new();
        let kzip_entries = &zip_archive.entries();
        // Store file names and their entry indexes in a hashmap
        for (i, entry) in kzip_entries.iter().enumerate() {
            if let EntryContents::File(_) = entry.contents() {
                if entry.name().contains("files/") {
                    file_map.insert(entry.name().replace("files/", ""), i);
                }
            }
        }
        Ok(Self { file, file_map, zip_archive })
    }

    /// Retrieve the Compilation Units from the kzip.
    ///
    /// This function will attempt to parse the proto files in the `pbunits`
    /// folder into Compilation Units, returning a Vector of them.
    ///
    /// # Errors
    ///
    /// If a file cannot be parsed, a
    /// [ProtobufParseError][KytheError::ProtobufParseError] will be returned.
    pub fn get_compilation_units(&self) -> Result<Vec<CompilationUnit>, KytheError> {
        let mut compilation_units = Vec::new();
        for entry in self.zip_archive.entries() {
            // Protobuf files are in the pbunits folder
            if let EntryContents::File(f) = entry.contents() {
                // Binary operations with `if let` are not currently supported,
                // so we must have an inner if-statement
                if entry.name().contains("/pbunits/") {
                    // Read the file into a compilation unit protobuf
                    let mut entry_reader =
                        f.entry.reader(|offset| positioned_io::Cursor::new_pos(&self.file, offset));
                    let bundle =
                        protobuf::parse_from_reader::<CompilationBundle>(&mut entry_reader)?;
                    compilation_units.push(bundle.get_unit().clone());
                }
            }
        }
        Ok(compilation_units)
    }
}

impl FileProvider for KzipFileProvider {
    /// Given a file hash, returns whether the file exists in the kzip.
    fn exists(&mut self, file_hash: &str) -> bool {
        self.file_map.contains_key(file_hash)
    }

    /// Given a file hash, returns the string contents of the file from the
    /// kzip.
    ///
    /// # Errors
    ///
    /// If the file does not exist in the kzip, a
    /// [FileNotFoundError][KytheError::FileNotFoundError] is returned.
    /// If the file cannot be read into a string, a
    /// [FileReadError][KytheError::FileReadError] is returned.
    fn contents(&mut self, file_hash: &str) -> Result<String, KytheError> {
        // Ensure the file exists in the kzip
        let position = self.file_map.get(file_hash).ok_or(KytheError::FileNotFoundError)?;
        let entries = &self.zip_archive.entries();
        // We clone here because a dereference would transfer ownership
        let entry = &entries[position.clone()];
        let mut reader = entry.reader(|offset| positioned_io::Cursor::new_pos(&self.file, offset));

        let mut file_contents = String::new();
        reader.read_to_string(&mut file_contents)?;
        Ok(file_contents)
    }
}
