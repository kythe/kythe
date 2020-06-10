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
use positioned_io;
use rc_zip::{prelude::*, Archive, EntryContents};
use std::fs::File;
use std::io::Read;

/// A trait for retrieving files during indexing.
pub trait FileProvider {
  /// Checks whether a file exists and that the contents is available.
  ///
  /// This function takes a file hash and returns whether
  /// the file exists and that the contents can be requested.
  ///
  /// # Errors
  ///
  /// If the file is not a valid zip archive, a [KytheError::KzipFileError] will
  /// be returned.
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
  zip_archive: Archive,
}

impl KzipFileProvider {
  /// Create a new instance using a valid [File] object of a kzip
  /// file.KytheError
  ///
  /// # Errors
  ///
  /// If the file is not a valid zip archive, a
  /// [KzipFileError][KytheError::KzipFileError] will be returned.
  pub fn new(file: File) -> Result<Self, KytheError> {
    let zip_archive = match file.read_zip() {
      Ok(zip_archive) => zip_archive,
      Err(_e) => return Err(KytheError::KzipFileError),
    };
    Ok(Self { file, zip_archive })
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
  pub fn get_compilation_units(
    &self,
  ) -> Result<Vec<CompilationUnit>, KytheError> {
    let mut compilation_units = Vec::new();
    for entry in self.zip_archive.entries() {
      // Protobuf files are in the pbunits folder
      if entry.name().contains("/pbunits/") {
        // Read the file into a compilation unit protobuf
        match entry.contents() {
          EntryContents::File(f) => {
            let mut entry_reader = f.entry.reader(|offset| {
              positioned_io::Cursor::new_pos(&self.file, offset)
            });
            let bundle = match ::protobuf::parse_from_reader::<CompilationBundle>(
              &mut entry_reader,
            ) {
              Ok(bundle) => bundle,
              Err(_e) => return Err(KytheError::ProtobufParseError),
            };
            compilation_units.push(bundle.get_unit().clone());
          }
          _ => {}
        };
      }
    }
    Ok(compilation_units)
  }

  /// Retrieve a file's location in the kzip
  ///
  /// This function takes a file hash and returns an Option of the file's index
  /// in the zip archive's file entries.
  fn get_file_position(&self, file_hash: &str) -> Option<usize> {
    self.zip_archive.entries().iter().position(|file_entry| {
      let file_path = format!("files/{}", file_hash);
      // The match ensures that the entry is a file instead of a directory or
      // symlink
      match file_entry.contents() {
        EntryContents::File(_) => file_entry.name().contains(&file_path),
        _ => false,
      }
    })
  }
}

impl FileProvider for KzipFileProvider {
  /// Given a file hash, returns whether the file exists in the kzip.
  fn exists(&mut self, file_hash: &str) -> bool {
    self.get_file_position(file_hash).is_some()
  }

  /// Given a file hash, returns the string contents of the file from the kzip.
  ///
  /// # Errors
  ///
  /// If the file does not exist in the kzip, a
  /// [FileNotFoundError][KytheError::FileNotFoundError] is returned.
  /// If the file cannot be read into a string, a
  /// [FileReadError][KytheError::FileReadError] is returned.
  fn contents(&mut self, file_hash: &str) -> Result<String, KytheError> {
    // Ensure file exists in the kzip
    if let Some(position) = self.get_file_position(file_hash) {
      let entries = &self.zip_archive.entries();
      let entry = &entries[position];
      let mut reader = entry
        .reader(|offset| positioned_io::Cursor::new_pos(&self.file, offset));

      let mut file_contents = String::new();
      match reader.read_to_string(&mut file_contents) {
        Ok(_) => Ok(file_contents),
        Err(_e) => Err(KytheError::FileReadError),
      }
    } else {
      Err(KytheError::FileNotFoundError)
    }
  }
}
