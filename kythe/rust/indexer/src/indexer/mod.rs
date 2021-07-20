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
pub mod save_analysis;

use crate::error::KytheError;
use crate::providers::FileProvider;
use crate::writer::KytheWriter;

use analysis_rust_proto::*;
use analyzers::UnitAnalyzer;
use std::path::Path;

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
        analysis_dir: &Path,
        provider: &mut dyn FileProvider,
    ) -> Result<(), KytheError> {
        let mut generator = UnitAnalyzer::new(unit, self.writer, provider);

        // First, create file nodes for all of the source files in the CompilationUnit
        generator.handle_files()?;

        // Then, index all of the crates from the save_analysis
        let analyzed_crates = save_analysis::load_analysis(&analysis_dir);
        for krate in analyzed_crates {
            generator.index_crate(krate)?;
        }

        // We must flush the writer each time to ensure that all entries get written
        self.writer.flush()?;
        Ok(())
    }
}
