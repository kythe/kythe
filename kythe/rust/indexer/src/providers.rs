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
use std::fs::File;
use std::io::{BufReader, Read};
use std::path::Path;
use zip::ZipArchive;

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

    /// Retrieves the byte contents of a file.
    ///
    /// This function takes a file hash and returns the bytes contents of the
    /// file in a vector.
    ///
    /// # Errors
    ///
    /// If the file does not exist, a
    /// [FileNotFoundError][KytheError::FileNotFoundError] will be returned.
    /// If the file can't be read, a [FileReadError][KytheError::FileReadError]
    /// will be returned.
    fn contents(&mut self, file_hash: &str) -> Result<Vec<u8>, KytheError>;
}

/// A [FileProvider] that backed by a .kzip file.
pub struct KzipFileProvider {
    zip_archive: ZipArchive<BufReader<File>>,
    /// The top level folder name inside of the zip archive
    root_name: String,
}

impl KzipFileProvider {
    /// Create a new instance using a valid [File] object of a kzip
    /// file.
    ///
    /// # Errors
    ///
    /// If an error occurs while reading the kzip, a [KzipFileError] will be
    /// returned.
    pub fn new(file: File) -> Result<Self, KytheError> {
        let reader = BufReader::new(file);
        let mut zip_archive = ZipArchive::new(reader)?;

        // Get the name of the root folder of the kzip. This should be almost always be
        // "root" by the kzip spec doesn't guarantee it.
        let root_name = {
            let file = zip_archive.by_index(0)?;
            let mut path = Path::new(file.name());
            while let Some(p) = path.parent() {
                // Last parent will be blank path so break when p is empty. path will contain
                // root
                if p == Path::new("") {
                    break;
                }
                path = p;
            }
            // Safe to unwrap because kzip read would have failed if the internal paths
            // weren't UTF-8
            path.to_str().unwrap().to_owned()
        };

        Ok(Self { zip_archive, root_name })
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
    pub fn get_compilation_units(&mut self) -> Result<Vec<CompilationUnit>, KytheError> {
        let mut compilation_units = Vec::new();
        for i in 0..self.zip_archive.len() {
            // Protobuf files are in the pbunits folder
            let file = self.zip_archive.by_index(i)?;
            if file.is_file() && file.name().contains("/pbunits/") {
                let mut reader = BufReader::new(file);
                let bundle = protobuf::parse_from_reader::<CompilationBundle>(&mut reader)?;
                compilation_units.push(bundle.get_unit().clone());
            }
        }
        Ok(compilation_units)
    }
}

impl FileProvider for KzipFileProvider {
    /// Given a file hash, returns whether the file exists in the kzip.
    fn exists(&mut self, file_hash: &str) -> bool {
        let name = format!("{}/files/{}", self.root_name, file_hash);
        self.zip_archive.by_name(&name).is_ok()
    }

    /// Given a file hash, returns the vector of bytes of the file from the
    /// kzip.
    ///
    /// # Errors
    ///
    /// An error will be returned if the file does not exist or cannot be read.
    fn contents(&mut self, file_hash: &str) -> Result<Vec<u8>, KytheError> {
        // Ensure the file exists in the kzip
        let name = format!("{}/files/{}", self.root_name, file_hash);
        let file = self.zip_archive.by_name(&name).map_err(|_| KytheError::FileNotFoundError)?;
        let mut reader = BufReader::new(file);
        let mut file_contents: Vec<u8> = Vec::new();
        reader.read_to_end(&mut file_contents)?;
        Ok(file_contents)
    }
}
