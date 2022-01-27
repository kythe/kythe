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

pub mod analyzers;
pub mod entries;
pub mod offset;

use crate::error::KytheError;
use crate::providers::FileProvider;
use crate::writer::KytheWriter;

use analysis_rust_proto::*;
use analyzers::UnitAnalyzer;
use std::path::PathBuf;

/// A data structure for indexing CompilationUnits
pub struct KytheIndexer<'a> {
    writer: &'a mut dyn KytheWriter,
}

impl<'a> KytheIndexer<'a> {
    /// Create a new instance of the KytheIndexer
    pub fn new(writer: &'a mut dyn KytheWriter) -> Self {
        Self { writer }
    }

    /// Accepts a CompilationUnit and the directory for analysis files and
    /// indexes the CompilationUnit
    pub fn index_cu(
        &mut self,
        unit: &CompilationUnit,
        provider: &mut dyn FileProvider,
        emit_std_lib: bool,
    ) -> Result<(), KytheError> {
        let analysis_file = Self::get_analysis_file(unit, provider)?;
        let mut generator = UnitAnalyzer::new(unit, self.writer, provider)?;

        if let Some(analysis) = rls_analysis::deserialize_crate_data(&analysis_file) {
            // First, create file nodes for all of the source files in the CompilationUnit
            generator.handle_files()?;
            // Then index the crate
            generator.index_crate(analysis, emit_std_lib)?;
        } else {
            return Err(KytheError::IndexerError(
                "Failed to deserialize save-analysis file".to_string(),
            ));
        }

        // We must flush the writer each time to ensure that all entries get written
        self.writer.flush()?;
        Ok(())
    }

    fn get_analysis_file(
        c_unit: &CompilationUnit,
        provider: &mut dyn FileProvider,
    ) -> Result<String, KytheError> {
        for required_input in c_unit.get_required_input() {
            let input_path = required_input.get_info().get_path();
            let input_path_buf = PathBuf::from(input_path);

            // save_analysis files are JSON files
            if let Some(os_str) = input_path_buf.extension() {
                if let Some("json") = os_str.to_str() {
                    let hash = required_input.get_info().get_digest();
                    let file_bytes = provider.contents(input_path, hash)?;
                    let file_string = String::from_utf8_lossy(&file_bytes);
                    return Ok(file_string.to_string());
                }
            }
        }
        Err(KytheError::IndexerError(
            "The save-analysis file could not be found in the Compilation Unit".to_string(),
        ))
    }
}
