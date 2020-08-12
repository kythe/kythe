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
use super::offset::OffsetIndex;

use analysis_rust_proto::CompilationUnit;
use rls_analysis::Crate;
use rls_data::{Def, DefKind};
use std::collections::HashMap;
use std::ffi::OsStr;
use std::fs;
use std::path::{Path, PathBuf};
use storage_rust_proto::*;

/// A data structure to analyze and index CompilationUnit protobufs
pub struct UnitAnalyzer<'a> {
    // The CompilationUnit being analyzed
    unit: &'a CompilationUnit,
    // The storage_rust_proto VName for the CompilationUnit
    unit_storage_vname: VName,
    // The emitter used to  write generated nodes and edges
    emitter: EntryEmitter<'a>,
    // The root directory for the source files
    root_dir: &'a PathBuf,
    // A map between a file name and it's Kythe VName
    file_vnames: HashMap<String, VName>,
    // An index for computing byte offsets in files based on line and column number
    offset_index: OffsetIndex,
}

/// A data structure to analyze and index individual crates
pub struct CrateAnalyzer<'a, 'b> {
    // The emitter used to  write generated nodes and edges
    emitter: &'b mut EntryEmitter<'a>,
    // A map between a file name and it's Kythe VName
    file_vnames: &'b HashMap<String, VName>,
    // The current CompilationUnit's VName
    unit_vname: &'b VName,
    // The current crate being analyzed
    krate: Crate,
    // A map between a crate's identifier and a string consisting of
    // "<disambiguator1>_<disambiguator2>"
    krate_ids: HashMap<u32, String>,
    // The crate's VName
    krate_vname: VName,
    // Stores the parent of a child definition so that a childof edge can be emitted when the
    // child's definition is analyzed
    children_ids: HashMap<rls_data::Id, VName>,
    // Stores VNames for emitted definitions based on definition Id
    definition_vnames: HashMap<rls_data::Id, VName>,
    // An index for computing byte offsets in files based on line and column number
    offset_index: &'b OffsetIndex,
    // An index for mapping a method's definition Id to what it is implementing
    method_index: HashMap<rls_data::Id, MethodImpl>,
    // A map from type names to their vnames
    type_vnames: HashMap<String, VName>,
}

/// A data struct to keep track of method implementations. Used in a HashMap to
/// map a method definition Id to it's struct and corresponding trait.
pub struct MethodImpl {
    // The struct definition Id the method is being implemented on
    pub struct_target: rls_data::Id,
    // The trait definition Id, if any, this is being implemented for
    pub trait_target: Option<rls_data::Id>,
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
        Self {
            unit,
            unit_storage_vname,
            emitter: EntryEmitter::new(writer),
            root_dir,
            file_vnames,
            offset_index: OffsetIndex::default(),
        }
    }

    /// Emits file nodes for all of the source files in a CompilationUnit and
    /// generate the OffsetIndex
    pub fn handle_files(&mut self) -> Result<(), KytheError> {
        // https://kythe.io/docs/schema/#file
        for source_file in self.unit.get_source_file() {
            let vname = self.get_file_vname(source_file)?;

            // Create the file node fact
            self.emitter.emit_node(&vname, "/kythe/node/kind", b"file".to_vec())?;

            // Create language fact
            self.emitter.emit_node(&vname, "/kythe/language", b"rust".to_vec())?;

            // Read the file contents and set it on the fact
            // Returns a FileReadError if we can't read the file
            let file_contents = fs::read_to_string(self.root_dir.join(Path::new(&source_file)))?;

            // Add the file to the OffsetIndex
            self.offset_index.add_file(&source_file, &file_contents);

            // Create text fact
            self.emitter.emit_node(&vname, "/kythe/text", file_contents.into_bytes())?;
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
            &self.offset_index,
        );
        crate_analyzer.emit_crate_nodes()?;
        crate_analyzer.emit_tbuiltin_nodes()?;
        crate_analyzer.process_implementations()?;
        crate_analyzer.emit_definitions()?;
        crate_analyzer.emit_xrefs()?;
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
        offset_index: &'b OffsetIndex,
    ) -> Self {
        // Initialize the type_vnames HashMap with builtin types
        let types = vec![
            "array",
            "bool",
            "char",
            "closure",
            "enum",
            "f32",
            "f64",
            "i8",
            "i16",
            "i32",
            "i64",
            "i128",
            "isize",
            "never",
            "reference",
            "slice",
            "str",
            "trait",
            "tuple",
            "u8",
            "u16",
            "u32",
            "u64",
            "u128",
            "usize",
        ];
        let mut type_vnames: HashMap<String, VName> = HashMap::new();
        let mut vname_template = VName::new();
        vname_template.set_corpus("std".to_string());
        vname_template.set_root("".to_string());
        vname_template.set_path("".to_string());
        vname_template.set_language("rust".to_string());
        for t in types.iter() {
            let mut type_vname = vname_template.clone();
            type_vname.set_signature(format!("{}#builtin", t));
            type_vnames.insert(t.to_string(), type_vname);
        }

        Self {
            emitter,
            file_vnames,
            krate,
            unit_vname,
            krate_ids: HashMap::new(),
            krate_vname: VName::new(),
            children_ids: HashMap::new(),
            definition_vnames: HashMap::new(),
            offset_index,
            method_index: HashMap::new(),
            type_vnames,
        }
    }

    /// Given a signature, generates a VName for a crate based on the VName of
    /// the CompilationUnit
    fn generate_crate_vname(&self, signature: &str) -> VName {
        let mut crate_v_name = self.unit_vname.clone();
        crate_v_name.set_signature(signature.to_string());
        crate_v_name.set_language("rust".to_string());
        crate_v_name.clear_path();
        crate_v_name
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
        self.krate_vname = krate_vname.clone();
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

    /// Emits tbuiltin nodes for all of the Rust built-in types
    pub fn emit_tbuiltin_nodes(&mut self) -> Result<(), KytheError> {
        for vname in self.type_vnames.values() {
            self.emitter.emit_node(vname, "/kythe/node/kind", b"tbuiltin".to_vec())?;
        }

        Ok(())
    }

    /// Creates the internal `method_index` by analyzing the implementations and
    /// relations on the save_analysis.
    ///
    /// Must be called before "emit_definitions"
    pub fn process_implementations(&mut self) -> Result<(), KytheError> {
        // Create a HashMap mapping the implementation Id to the implementation
        // It might be a safe assumption that the index is the Id, but we can't be too
        // careful
        let impls = self.krate.analysis.impls.clone();
        let mut impl_map: HashMap<u32, rls_data::Impl> = HashMap::new();
        for implementation in impls.iter() {
            impl_map.insert(implementation.id, implementation.clone());
        }
        // Drop the cloned vector to save memory
        drop(impls);

        // Create a HashMap betwen a method definition Id and the struct and trait being
        // implemented on
        let mut method_index: HashMap<rls_data::Id, MethodImpl> = HashMap::new();
        let relations = &self.krate.analysis.relations;
        for relation in relations.iter() {
            // If this is an implementation relation
            if let rls_data::RelationKind::Impl { id: impl_id, .. } = relation.kind {
                let implementation = impl_map.get(&impl_id).ok_or_else(|| {
                    KytheError::IndexerError(format!(
                        "Couldn't find implementation for relation {:?}",
                        relation
                    ))
                })?;
                // Add all of the childred to the HashMap
                for child in implementation.children.iter() {
                    // The struct being implemented on
                    let struct_target = relation.from;
                    // The optional trait being implemented on
                    // The save_analysis file will have the krate and index be maxint if the
                    // implementation is not on a trait.
                    let max_int = 4294967295;
                    let trait_target =
                        if relation.to.krate != max_int { Some(relation.to) } else { None };
                    method_index.insert(*child, MethodImpl { struct_target, trait_target });
                }
            }
        }
        self.method_index = method_index;
        Ok(())
    }

    /// Given a definition for a module, returns the byte span
    /// for the module definition
    ///
    /// # Notes:
    /// If the definition provided isn't for a module, `false` is returned.
    /// If the span file name is called `mod.rs` but there is no parent
    /// directory, `false` is returned.
    fn is_module_implicit(&self, def: &Def) -> bool {
        // Ensure that this defition is for a module
        if def.kind != DefKind::Mod {
            return false;
        }

        // Check if this is the primary module of the crate. If so, the module starts at
        // the top of the file
        if def.qualname == "::" {
            return true;
        }

        let file_path = def.span.file_name.clone();

        // The name we expect if the module definition is the file itself
        let expected_name: String;

        // If the file name is mod.rs, then the expected name is the directory name
        if Some(OsStr::new("mod.rs")) == file_path.file_name() {
            if let Some(parent_directory) = file_path.parent() {
                expected_name = parent_directory.file_name().unwrap().to_str().unwrap().to_string();
            } else {
                // This should only happen if there is something wrong with the file path we
                // were provided in the CompilationUnit
                return false;
            }
        } else {
            // Get the file name without the extension and convert to a string
            expected_name = file_path.file_stem().unwrap().to_str().unwrap().to_string();
        }

        // If the names match, the module definition is implicit
        def.name == expected_name
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
            // Check for "./" at the beginning and remove it
            let file_vname = self.file_vnames.get(def.span.file_name.to_str().unwrap());

            // save_analysis sometimes references files that we have as file nodes
            if file_vname.is_none() {
                continue;
            }

            // Generate node based on definition type
            let mut def_vname = self.krate_vname.clone();
            let def_signature = format!("{}_def_{}", krate_signature, def.id.index);
            def_vname.set_signature(def_signature.clone());
            def_vname.set_language("rust".to_string());
            def_vname.clear_path();
            self.emit_definition_node(&def_vname, &def, &file_vname.unwrap())?;
        }

        // Normally you'd want to have a catch-all here where you emit childof edges
        // that may have been missed if the child's definition came before the parent's
        // definition. However, it is possible for the parent's children to contain an
        // Id that doesn't appear in the list of definitions (?) so we handle the "child
        // before parent" case in `emit_definition_node` and clear the HashMap
        // here to drop the orphans and save memory.
        self.children_ids.clear();

        for (child_id, parent_vname) in self.children_ids.iter() {
            let child_vname = self.definition_vnames.remove(child_id).ok_or_else(|| {
                KytheError::IndexerError(format!(
                    "Failed to get vname for child {:?} when emitting childof edge",
                    child_id
                ))
            })?;
            self.emitter.emit_edge(&child_vname, parent_vname, "/kythe/edge/childof")?;
        }

        Ok(())
    }

    /// Emit all of the Kythe graph nodes and edges, including anchors for the
    /// definition using the provided VName
    fn emit_definition_node(
        &mut self,
        def_vname: &VName,
        def: &Def,
        file_vname: &VName,
    ) -> Result<(), KytheError> {
        // For Fields, we always emit childof edges to their
        // parent because TupleVariant definitions don't have their fields as children.
        // Structs and Unions have their fields listed as children so the facts would
        // duplicate if we didn't have this `if` statement
        if def.kind != DefKind::Struct && def.kind != DefKind::Union {
            // Track children to emit childof nodes later
            for child in def.children.iter() {
                if let Some(child_vname) = self.definition_vnames.get(child) {
                    // The child definition has already been visited and we can emit the childof
                    // edge now
                    self.emitter.emit_edge(child_vname, def_vname, "/kythe/edge/childof")?;
                } else {
                    self.children_ids.insert(*child, def_vname.clone());
                }
            }
        }

        // Check if the current definition is a child of another node and remove the
        // entry if it existed
        if let Some(parent_vname) = self.children_ids.remove(&def.id) {
            self.emitter.emit_edge(def_vname, &parent_vname, "/kythe/edge/childof")?;
        }

        // Store the definition's VName so it can be referenced by its id later
        self.definition_vnames.insert(def.id, def_vname.clone());

        // (fact_name, fact_value)
        let mut facts: Vec<(&str, &[u8])> = Vec::new();

        // Generate the facts to be emitted and emit some edges as necessary
        match def.kind {
            DefKind::Const | DefKind::Static => {
                facts.push(("/kythe/node/kind", b"constant"));
            }
            DefKind::Enum => {
                facts.push(("/kythe/node/kind", b"sum"));
                facts.push(("/kythe/complete", b"definition"));
                facts.push(("/kythe/subkind", b"enum"));
            }
            DefKind::Field => {
                // TODO: Determine how to emit `typed` edges for field based on the `value` on
                // the definition
                facts.push(("/kythe/node/kind", b"variable"));
                facts.push(("/kythe/complete", b"definition"));
                facts.push(("/kythe/subkind", b"field"));

                if let Some(parent_id) = def.parent {
                    // Field definitions come after their parent's definitions so their VName should
                    // be in the list of VNames
                    let parent_vname = self.definition_vnames.get(&parent_id).ok_or_else(|| {
                        KytheError::IndexerError(format!(
                            "Failed to get vname for parent of definition {:?}",
                            def.id
                        ))
                    })?;
                    // Emit the childof edge between this node and the parent
                    self.emitter.emit_edge(def_vname, parent_vname, "/kythe/edge/childof")?;
                }
            }
            DefKind::Function => {
                // TODO: Handle parameters, emit a defines edge on the entire definition span of
                // the function
                facts.push(("/kythe/node/kind", b"function"));
                facts.push(("/kythe/complete", b"definition"));
            }
            DefKind::Local => {
                // If the variable is a closure, emit that it is a function
                if def.value.contains("closure@") {
                    facts.push(("/kythe/node/kind", b"function"));
                    facts.push(("/kythe/complete", b"definition"));
                } else {
                    facts.push(("/kythe/node/kind", b"variable"));
                    facts.push(("/kythe/subkind", b"local"));
                }

                // TODO: Find a way to determine if a variable is only being
                // declared. The save_analysis uses different index offsets for
                // difference classes of local variables. One class is for
                // mutable variables, one is for immutable variables, and one is
                // for declarations. However these aren't static offsets, they
                // are based on how many classes were previously seen. The first
                // class that is seen has an offset of ~2147483647, the second
                // has an offset of ~1073741827, and the third has an offset of
                // ~3221225471.
            }
            DefKind::Method => {
                // TODO: Handle parameters, emit a defines edge on the entire definition span of
                // the method
                facts.push(("/kythe/node/kind", b"function"));
                facts.push(("/kythe/complete", b"definition"));

                // If the if-statement logic passes, this is a method on a struct and we emit a
                // "childof" edge. Otherwise, it is a method on a trait and we
                // ignore it
                if let Some(method_impl) = self.method_index.remove(&def.id) {
                    // Get the vname of the parent struct
                    // Need to have the option variable to avoid multiple borrows on "self".
                    let parent_vname_option =
                        self.definition_vnames.get(&method_impl.struct_target);
                    let parent_vname = if let Some(vname) = parent_vname_option {
                        vname.clone()
                    } else {
                        // Create a default VName based on the krate's vname and use the target's
                        // krate disambiguator and index to create the signature. Usually this
                        // occurs if the save_analysis feeds us data that isn't part of our crate.
                        let mut vname = self.krate_vname.clone();
                        let disambiguator = self
                            .krate_ids
                            .get(&method_impl.struct_target.krate)
                            .ok_or_else(|| {
                                KytheError::IndexerError(format!(
                                    "Can't find parent vname for method {:?} (\"{}\")",
                                    def.id, def.name
                                ))
                            })?;
                        vname.set_signature(format!(
                            "{}_def_{}",
                            disambiguator, &method_impl.struct_target.index
                        ));
                        vname
                    };
                    // Emit a childof edge to the parent struct
                    self.emitter.emit_edge(def_vname, &parent_vname, "/kythe/edge/childof")?;
                }
            }
            DefKind::Mod => {
                facts.push(("/kythe/node/kind", b"record"));
                facts.push(("/kythe/subkind", b"module"));
                facts.push(("/kythe/complete", b"definition"));
                // Emit the childof edge on the crate if this is the main module
                if def.qualname == "::" {
                    self.emitter.emit_edge(def_vname, &self.krate_vname, "/kythe/edge/childof")?;
                }
            }
            DefKind::Struct => {
                facts.push(("/kythe/node/kind", b"record"));
                facts.push(("/kythe/complete", b"definition"));
                facts.push(("/kythe/subkind", b"struct"));
            }
            // Struct inside an enum
            DefKind::StructVariant => {
                facts.push(("/kythe/node/kind", b"record"));
                facts.push(("/kythe/complete", b"definition"));
                facts.push(("/kythe/subkind", b"structvariant"));
            }
            DefKind::Trait => {
                facts.push(("/kythe/node/kind", b"interface"));
            }
            // Tuple inside an enum. Has 0 or more parameters and includes constants
            DefKind::TupleVariant => {
                // Check if this is a constant inside of an enum
                if def.qualname.ends_with(&def.value) {
                    // TODO: Determine how to get the value of the constant.
                    // Currently might not be possible using the data from the
                    // save_analysis
                    facts.push(("/kythe/node/kind", b"constant"));
                } else {
                    facts.push(("/kythe/node/kind", b"record"));
                    facts.push(("/kythe/complete", b"definition"));
                    facts.push(("/kythe/subkind", b"tuplevariant"));
                }
            }
            DefKind::Type => {
                facts.push(("/kythe/node/kind", b"talias"));

                // If it aliases a builtin type, emit an aliases edge
                // TODO: Make this work with custom types
                if let Some(type_vname) = self.type_vnames.get(&def.value) {
                    self.emitter.emit_edge(def_vname, type_vname, "/kythe/edge/aliases")?;
                }
            }
            DefKind::Union => {
                facts.push(("/kythe/node/kind", b"record"));
                facts.push(("/kythe/complete", b"definition"));
                facts.push(("/kythe/subkind", b"union"));
            }
            // TODO(Arm1stice): Support other types of definitions
            _ => {}
        }

        // Emit nodes for all fact/value pairs
        for (fact_name, fact_value) in facts.iter() {
            self.emitter.emit_node(def_vname, fact_name, fact_value.to_vec())?;
        }

        // Calculate the byte_start and byte_end using the OffsetIndex
        let byte_start = self
            .offset_index
            .get_byte_offset(file_vname.get_path(), def.span.line_start.0, def.span.column_start.0)
            .ok_or_else(|| {
                KytheError::IndexerError(format!(
                    "Failed to get starting offset for definition {:?}",
                    def.id
                ))
            })?;
        let byte_end = self
            .offset_index
            .get_byte_offset(file_vname.get_path(), def.span.line_end.0, def.span.column_end.0)
            .ok_or_else(|| {
                KytheError::IndexerError(format!(
                    "Failed to get ending offset for definition {:?}",
                    def.id
                ))
            })?;

        let mut anchor_vname = def_vname.clone();
        anchor_vname.set_signature(format!("{}_anchor", def_vname.get_signature()));
        anchor_vname.set_path(file_vname.get_path().to_string());

        // Module definitions need special logic if they are implicit
        if def.kind == DefKind::Mod && self.is_module_implicit(def) {
            // Emit a 0-length anchor and defines edge at the top of the file
            self.emitter.emit_node(&anchor_vname, "/kythe/node/kind", b"anchor".to_vec())?;
            self.emitter.emit_node(&anchor_vname, "/kythe/loc/start", b"0".to_vec())?;
            self.emitter.emit_node(&anchor_vname, "/kythe/loc/end", b"0".to_vec())?;
            self.emitter.emit_edge(&anchor_vname, def_vname, "/kythe/edge/defines/implicit")?;
        } else {
            self.emitter.emit_anchor(&anchor_vname, def_vname, byte_start, byte_end)?;
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

    /// Emit the Kythe edges for cross references in this crate
    pub fn emit_xrefs(&mut self) -> Result<(), KytheError> {
        // We must clone to avoid double borrowing "self"
        let analysis = self.krate.analysis.clone();

        let mut template_vname = self.krate_vname.clone();
        template_vname.set_language("rust".to_string());
        let krate_signature = template_vname.get_signature();

        for reference in &analysis.refs {
            let mut reference_vname = template_vname.clone();
            let span = &reference.span;
            reference_vname.set_path(span.file_name.to_str().unwrap().to_string());

            // Get byte span
            let start_byte_option = self.offset_index.get_byte_offset(
                span.file_name.to_str().unwrap(),
                span.line_start.0,
                span.column_start.0,
            );

            // If the start byte is none, then the save_analysis is giving information about
            // standard library files and we should skip
            if start_byte_option.is_none() {
                continue;
            }

            let start_byte = start_byte_option.unwrap();
            let end_byte = self
                .offset_index
                .get_byte_offset(
                    span.file_name.to_str().unwrap(),
                    span.line_end.0,
                    span.column_end.0,
                )
                .ok_or_else(|| {
                    KytheError::IndexerError(format!(
                        "Failed to get ending offset for reference {:?}",
                        reference
                    ))
                })?;

            // Create signature based on span
            reference_vname
                .set_signature(format!("{}_ref_{}_{}", krate_signature, start_byte, end_byte));

            // Create VName being referenced
            let disambiguators = self.krate_ids.get(&reference.ref_id.krate).ok_or_else(|| {
                KytheError::IndexerError(format!(
                    "Failed to get krate disambiguator for reference {:?}",
                    reference
                ))
            })?;

            let mut target_vname = template_vname.clone();
            target_vname
                .set_signature(format!("{}_def_{}", disambiguators, reference.ref_id.index));
            target_vname.set_language("rust".to_string());

            self.emitter.emit_reference(&reference_vname, &target_vname, start_byte, end_byte)?;
        }
        Ok(())
    }
}

/// Convert a VName from analysis_rust_proto to a VName from storage_rust_proto
fn analysis_to_storage_vname(analysis_vname: &analysis_rust_proto::VName) -> VName {
    let mut vname = VName::new();
    vname.set_signature(analysis_vname.get_signature().to_string());
    vname.set_corpus(analysis_vname.get_corpus().to_string());
    vname.set_root(analysis_vname.get_root().to_string());
    vname.set_path(analysis_vname.get_path().to_string());
    vname.set_language(analysis_vname.get_signature().to_string());
    vname
}
