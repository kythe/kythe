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
use crate::writer::KytheWriter;

use super::entries::EntryEmitter;

use analysis_rust_proto::CompilationUnit;
use rls_analysis::Crate;
use rls_data::{Def, DefKind};
use std::collections::HashMap;
use std::fs::File;
use std::io::Read;
use std::path::{Path, PathBuf};
use storage_rust_proto::*;

/// A data structure to analyze and index CompilationUnit protobufs
pub struct UnitAnalyzer<'a> {
    unit: &'a CompilationUnit,
    unit_storage_vname: VName,
    emitter: EntryEmitter<'a>,
    root_dir: &'a PathBuf,
    file_vnames: HashMap<String, VName>,
}

/// A data structure to analyze and index individual crates
pub struct CrateAnalyzer<'a, 'b> {
    emitter: &'b mut EntryEmitter<'a>,
    file_vnames: &'b HashMap<String, VName>,
    unit_vname: &'b VName,
    krate: Crate,
    krate_ids: HashMap<u32, String>,
}

impl<'a> UnitAnalyzer<'a> {
    /// Create an instance to assist in analyzing `unit`. Graph information will
    /// be written to the `writer` and source file contents will be read using
    /// `root_dir` as a base directory.
    pub fn new(
        unit: &'a CompilationUnit,
        writer: &'a mut dyn KytheWriter,
        root_dir: &'a PathBuf,
    ) -> Self {
        // Create a HashMap between the file path and the VName which we can retrieve
        // later to emit nodes
        let mut file_vnames = HashMap::new();
        for required_input in unit.get_required_input() {
            let analysis_vname = required_input.get_v_name();
            let storage_vname: VName = analysis_to_storage_vname(&analysis_vname);
            let path = storage_vname.get_path().to_owned();
            file_vnames.insert(path, storage_vname);
        }

        let unit_storage_vname: VName = analysis_to_storage_vname(&unit.get_v_name());
        Self { unit, unit_storage_vname, emitter: EntryEmitter::new(writer), root_dir, file_vnames }
    }

    /// Emits file nodes for all of the source files in a CompilationUnit
    pub fn emit_file_nodes(&mut self) -> Result<(), KytheError> {
        // https://kythe.io/docs/schema/#file
        for source_file in self.unit.get_source_file() {
            let vname = self.get_file_vname(source_file)?;

            // Create the file node fact
            self.emitter.emit_node(&vname, "/kythe/node/kind", b"file".to_vec())?;

            // Create language fact
            self.emitter.emit_node(&vname, "/kythe/language", b"rust".to_vec())?;

            // Read the file contents and set it on the fact
            // Returns a FileReadError if we can't read the file
            let mut file = File::open(self.root_dir.join(Path::new(&source_file)))?;
            let mut file_contents: Vec<u8> = Vec::new();
            file.read_to_end(&mut file_contents)?;

            // Create text fact
            self.emitter.emit_node(&vname, "/kythe/text", file_contents)?;
        }
        Ok(())
    }

    /// Indexes the provided crate
    pub fn index_crate(&mut self, krate: Crate) -> Result<(), KytheError> {
        let mut crate_analyzer = CrateAnalyzer::new(
            &mut self.emitter,
            &self.file_vnames,
            &self.unit_storage_vname,
            krate,
        );
        crate_analyzer.emit_crate_nodes()?;
        crate_analyzer.emit_definitions()?;
        // TODO(Arm1stice): Emit references and implementations
        Ok(())
    }

    /// Given a file name, returns a [Result] with the file's VName from the
    /// Compilation Unit.
    ///
    /// # Errors
    /// If the file name isn't found, a [KytheError::IndexerError] is returned.
    fn get_file_vname(&mut self, file_name: &str) -> Result<VName, KytheError> {
        let err_msg = format!(
            "Failed to find VName for file \"{}\" located in the save analysis. Is it included in the required inputs of the Compilation Unit?",
            file_name
        );
        let vname = self.file_vnames.get(file_name).ok_or(KytheError::IndexerError(err_msg))?;
        Ok(vname.clone())
    }
}

impl<'a, 'b> CrateAnalyzer<'a, 'b> {
    /// Create a new instance to analyze `krate` which will emit to `emitter`
    /// and use `file_vnames` to resolve VNames from file names
    pub fn new(
        emitter: &'b mut EntryEmitter<'a>,
        file_vnames: &'b HashMap<String, VName>,
        unit_vname: &'b VName,
        krate: Crate,
    ) -> Self {
        Self { emitter, file_vnames, krate, unit_vname, krate_ids: HashMap::new() }
    }

    /// Generates and emits package nodes for the main crate and external crates
    /// NOTE: Must be called first to populate the self.krate_ids HashMap
    pub fn emit_crate_nodes(&mut self) -> Result<(), KytheError> {
        let krate_analysis = &self.krate.analysis;
        let krate_prelude = &krate_analysis.prelude.as_ref().ok_or_else(|| {
            KytheError::IndexerError(format!(
                "Crate \"{}\" did not have prelude data",
                &self.krate.id.name
            ))
        })?;

        // First emit the node for our own crate and add it to the hashmap
        let krate_id = &krate_prelude.crate_id;
        let krate_signature = format!("{}_{}", krate_id.disambiguator.0, krate_id.disambiguator.1);
        let krate_vname = self.generate_crate_vname(&krate_signature);
        self.emitter.emit_node(&krate_vname, "/kythe/node/kind", b"package".to_vec())?;
        self.krate_ids.insert(0u32, krate_signature);

        // Then, do the same for all of the external crates
        for (krate_num, external_krate) in krate_prelude.external_crates.iter().enumerate() {
            let krate_id = &external_krate.id;
            let krate_signature =
                format!("{}_{}", krate_id.disambiguator.0, krate_id.disambiguator.1);
            let krate_vname = self.generate_crate_vname(&krate_signature);
            self.emitter.emit_node(&krate_vname, "/kythe/node/kind", b"package".to_vec())?;
            self.krate_ids.insert((krate_num + 1) as u32, krate_signature);
        }

        Ok(())
    }

    /// Emit Kythe graph information for the definitions in the crate
    pub fn emit_definitions(&mut self) -> Result<(), KytheError> {
        // We must clone to avoid double borrowing "self"
        let analysis = self.krate.analysis.clone();

        for def in &analysis.defs {
            let krate_signature = self.krate_ids.get(&def.id.krate).ok_or_else(||{
                KytheError::IndexerError(format!(
                    "Definition \"{}\" referenced crate \"{}\" which was not found in the krate_ids HashMap",
                    def.qualname, def.id.krate
                ))}
            )?;
            let file_vname =
                self.file_vnames.get(def.span.file_name.to_str().unwrap()).ok_or_else(|| {
                    KytheError::IndexerError(format!(
                        "Failed to get VName for file {:?} for definition {}",
                        def.span.file_name, def.qualname
                    ))
                })?;

            // Generate node based on definition type
            let mut def_vname = file_vname.clone();
            let def_signature = format!("{}_def_{}", krate_signature, def.id.index);
            def_vname.set_signature(def_signature.clone());
            def_vname.set_language("rust".to_string());
            self.emit_definition_node(&def_vname, &def)?;
        }

        Ok(())
    }

    /// Emit all of the Kythe graph nodes and edges, including anchors for the
    /// definition using the provided VName
    fn emit_definition_node(&mut self, def_vname: &VName, def: &Def) -> Result<(), KytheError> {
        let mut facts: HashMap<&str, Vec<u8>> = HashMap::new();

        // Generate the facts to be emitted
        // TODO(Arm1stice): Remove once more definitions are added to the match
        // statement
        #[allow(clippy::single_match)]
        match def.kind {
            DefKind::Function => {
                facts.insert("/kythe/node/kind", b"function".to_vec());
                facts.insert("/kythe/complete", b"definition".to_vec());
            }
            // TODO(Arm1stice): Support other types of definitions
            _ => {}
        }

        // Emit nodes for all fact/value pairs
        for (fact_name, fact_value) in &facts {
            self.emitter.emit_node(def_vname, fact_name, fact_value.clone())?;
        }

        let mut anchor_vname = def_vname.clone();
        anchor_vname.set_signature(format!("{}_anchor", def_vname.get_signature()));
        // If the definition is a Mod, place the anchor at the first byte only,
        // otherwise use the entire span
        // TODO(Arm1stice): Improve this mechanism to be more accurate for modules
        // defined inside of files
        if def.kind == DefKind::Mod {
            self.emitter.emit_anchor(
                &anchor_vname,
                def_vname,
                def.span.byte_start,
                def.span.byte_start + 1,
            )?;
        } else {
            self.emitter.emit_anchor(
                &anchor_vname,
                def_vname,
                def.span.byte_start,
                def.span.byte_end,
            )?;
        }

        // If documentation isn't "" also generate a documents node
        // - Emit documentation type node
        // - Emit documents edge from node to def
        if def.docs != "" {
            let mut doc_vname = def_vname.clone();
            let doc_signature = format!("{}_doc", def_vname.get_signature());
            doc_vname.set_signature(doc_signature);
            self.emitter.emit_node(&doc_vname, "/kythe/node/kind", b"doc".to_vec())?;
            self.emitter.emit_node(
                &doc_vname,
                "/kythe/text",
                def.docs.trim().as_bytes().to_vec(),
            )?;
            self.emitter.emit_edge(&doc_vname, def_vname, "/kythe/edge/documents")?;
        }

        Ok(())
    }

    /// Given a signature, generates a VName for a crate based on the VName of
    /// the CompilationUnit
    fn generate_crate_vname(&self, signature: &str) -> VName {
        let mut crate_v_name = self.unit_vname.clone();
        crate_v_name.set_signature(signature.to_string());
        crate_v_name.set_language("rust".to_string());
        crate_v_name
    }
}

// Convert a VName from analysis_rust_proto to a VName from storage_rust_proto
fn analysis_to_storage_vname(analysis_vname: &analysis_rust_proto::VName) -> VName {
    let mut vname = VName::new();
    vname.set_signature(analysis_vname.get_signature().to_string());
    vname.set_corpus(analysis_vname.get_corpus().to_string());
    vname.set_root(analysis_vname.get_root().to_string());
    vname.set_path(analysis_vname.get_path().to_string());
    vname.set_language(analysis_vname.get_signature().to_string());
    vname
}
