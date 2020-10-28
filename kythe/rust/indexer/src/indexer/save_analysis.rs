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

//! The purpose of this module is to implement functionality to recompile the
//! CompilationUnit and generate a save analysis that can be used by the rest of
//! the indexer to generate Kythe graph nodes.

use rls_analysis::{AnalysisHost, AnalysisLoader, Crate, SearchDirectory};
use std::path::{Path, PathBuf};

// Loader struct pulled from the rls_analysis example code: https://git.io/JfjAn
#[derive(Clone)]
pub struct Loader {
    deps_dir: PathBuf,
}

impl Loader {
    pub fn new(deps_dir: PathBuf) -> Self {
        Self { deps_dir }
    }
}

impl AnalysisLoader for Loader {
    fn needs_hard_reload(&self, _: &Path) -> bool {
        true
    }

    fn fresh_host(&self) -> AnalysisHost<Self> {
        AnalysisHost::new_with_loader(self.clone())
    }

    fn set_path_prefix(&mut self, _: &Path) {}

    fn abs_path_prefix(&self) -> Option<PathBuf> {
        None
    }
    fn search_directories(&self) -> Vec<SearchDirectory> {
        vec![SearchDirectory { path: self.deps_dir.clone(), prefix_rewrite: None }]
    }
}

/// Takes a PathBuf and loads the save_analysis files from the path
pub fn load_analysis(root_dir: &PathBuf) -> Vec<Crate> {
    let path = (*root_dir).clone();
    let loader = Loader::new(path);
    rls_analysis::read_analysis_from_files(&loader, Default::default(), &[] as &[&str])
}
