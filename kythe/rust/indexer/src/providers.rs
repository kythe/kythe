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
use crate::proxyrequests;
use analysis_rust_proto::*;
use serde_json::Value;
use std::fs::File;
use std::io::{self, BufReader, Read, Write};
use std::path::Path;
use zip::ZipArchive;

/// A trait for retrieving files during indexing.
pub trait FileProvider {
    /// Checks whether a file exists and that the contents are available.
    ///
    /// This function takes a file path and digest, and returns whether
    /// the file exists and that the contents can be requested.
    ///
    /// # Errors
    ///
    /// If the file is not a valid zip archive, a [KytheError::KzipFileError]
    /// will be returned.
    fn exists(&mut self, path: &str, digest: &str) -> Result<bool, KytheError>;

    /// Retrieves the byte contents of a file.
    ///
    /// This function takes a file path and digest, and returns the bytes
    /// contents of the file in a vector.
    ///
    /// # Errors
    ///
    /// If the file does not exist, a
    /// [FileNotFoundError][KytheError::FileNotFoundError] will be returned.
    /// If the file can't be read, a [FileReadError][KytheError::FileReadError]
    /// will be returned.
    fn contents(&mut self, path: &str, digest: &str) -> Result<Vec<u8>, KytheError>;
}

/// A [FileProvider] that is backed by a .kzip file.
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
        // "root" but the kzip spec doesn't guarantee it.
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
            path.to_str().unwrap().to_owned().replace('/', "")
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
                let indexed_comp: IndexedCompilation =
                    protobuf::Message::parse_from_reader(&mut reader)?;
                compilation_units.push(indexed_comp.get_unit().clone());
            }
        }
        Ok(compilation_units)
    }
}

impl KzipFileProvider {
    /// Constructs the path within the archive for the given file.
    fn file_path(&self, _path: &str, digest: &str) -> String {
        format!("{}/files/{}", self.root_name, digest)
    }
}

impl FileProvider for KzipFileProvider {
    /// Given a file path and digest, returns whether the file exists in the
    /// kzip.
    fn exists(&mut self, _path: &str, digest: &str) -> Result<bool, KytheError> {
        let name = self.file_path(_path, digest);
        Ok(self.zip_archive.by_name(&name).is_ok())
    }

    /// Given a file path and digest, returns the vector of bytes of the file
    /// from the kzip.
    ///
    /// # Errors
    ///
    /// An error will be returned if the file does not exist or cannot be read.
    fn contents(&mut self, _path: &str, digest: &str) -> Result<Vec<u8>, KytheError> {
        // Ensure the file exists in the kzip
        let name = self.file_path(_path, digest);
        let file =
            self.zip_archive.by_name(&name).map_err(|_| KytheError::FileNotFoundError(name))?;
        let mut reader = BufReader::new(file);
        let mut file_contents: Vec<u8> = Vec::new();
        reader.read_to_end(&mut file_contents)?;
        Ok(file_contents)
    }
}

/// A [FileProvider] that backed by the analysis driver proxy
#[derive(Default)]
pub struct ProxyFileProvider {}

impl ProxyFileProvider {
    pub fn new() -> Self {
        Self {}
    }
}

impl ProxyFileProvider {
    fn file_request(&mut self, path: &str, digest: &str) -> Result<Value, KytheError> {
        // Submit the file request to the proxy
        let request = proxyrequests::file(path.to_string(), digest.to_string())?;
        println!("{request}");
        io::stdout()
            .flush()
            .map_err(|err| KytheError::IndexerError(format!("Failed to flush stdout: {err:?}")))?;

        // Read the response from the proxy
        let mut response_string = String::new();
        io::stdin().read_line(&mut response_string).map_err(|err| {
            KytheError::IndexerError(format!(
                "Failed to read proxy file response from stdin: {err:?}"
            ))
        })?;

        // Convert to json and extract information
        let response: Value = serde_json::from_str(&response_string).map_err(|err| {
            KytheError::IndexerError(format!(
                "Failed to read proxy file response from stdin: {err:?}"
            ))
        })?;
        if response["rsp"].as_str().unwrap() == "error" {
            // The file couldn't be found
            Err(KytheError::FileNotFoundError(path.to_string()))
        } else {
            Ok(response)
        }
    }
}

impl FileProvider for ProxyFileProvider {
    /// Given a file path and digest, returns whether the proxy can find the
    /// file.
    fn exists(&mut self, path: &str, digest: &str) -> Result<bool, KytheError> {
        // There is no API to check for a file's existence so we must request its
        // contents.
        match self.file_request(path, digest) {
            Ok(_) => Ok(true),
            Err(KytheError::FileNotFoundError(_)) => Ok(false),
            Err(e) => Err(e),
        }
    }

    /// Given a file path and digest, returns the vector of bytes of the file
    /// from the proxy.
    fn contents(&mut self, path: &str, digest: &str) -> Result<Vec<u8>, KytheError> {
        let response = self.file_request(path, digest)?;
        let args = &response["args"];
        // Retrieve base64 content and convert to a Vec<u8>
        let content_option = args["content"].as_str();
        if content_option.is_none() {
            return Ok(vec![]);
        }
        let content_base64: &str = content_option.unwrap();
        let content: Vec<u8> = base64::decode(content_base64).map_err(|err| {
            KytheError::IndexerError(format!(
                "Failed to convert base64 file contents response to bytes: {err:?}"
            ))
        })?;
        Ok(content)
    }
}
