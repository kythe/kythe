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

use storage_rust_proto::*;

/// A utility data structure for writing nodes and edges to a KytheWriter
pub struct EntryEmitter<'a> {
    writer: &'a mut dyn KytheWriter,
}

impl<'a> EntryEmitter<'a> {
    /// Create a new instance using the provided KytheWriter
    pub fn new(writer: &'a mut dyn KytheWriter) -> Self {
        Self { writer }
    }

    /// Creates a node with the provided arguments and emits it.
    ///
    /// # Errors
    /// If an error occurs while writing the entry, an error is returned.
    pub fn emit_node(
        &mut self,
        vname: &VName,
        fact_name: &str,
        fact_value: Vec<u8>,
    ) -> Result<(), KytheError> {
        let mut entry = Entry::new();
        entry.set_source(vname.clone());
        entry.set_fact_name(fact_name.into());
        entry.set_fact_value(fact_value);

        self.writer.write_entry(entry)
    }

    /// Creates an edge of edge_kind between the source and target and emits it.
    ///
    /// # Errors
    /// If an error occurs while writing the entry, an error is returned.
    pub fn emit_edge(
        &mut self,
        source: &VName,
        target: &VName,
        edge_kind: &str,
    ) -> Result<(), KytheError> {
        let mut entry = Entry::new();
        entry.set_source(source.clone());
        entry.set_target(target.clone());
        entry.set_edge_kind(edge_kind.into());
        entry.set_fact_name("/".into());

        self.writer.write_entry(entry)
    }

    /// Creates an anchor node with defines/binding edge to the target and emits
    /// it.
    ///
    /// # Errors
    /// If an error occurs while writing the entry, an error is returned.
    pub fn emit_anchor(
        &mut self,
        anchor_vname: &VName,
        target_vname: &VName,
        byte_start: u32,
        byte_end: u32,
    ) -> Result<(), KytheError> {
        self.emit_node(anchor_vname, "/kythe/node/kind", b"anchor".to_vec())?;
        self.emit_node(
            anchor_vname,
            "/kythe/loc/start",
            byte_start.to_string().into_bytes().to_vec(),
        )?;
        self.emit_node(
            anchor_vname,
            "/kythe/loc/end",
            byte_end.to_string().into_bytes().to_vec(),
        )?;
        self.emit_edge(anchor_vname, target_vname, "/kythe/edge/defines/binding")
    }

    /// Creates an anchor node with ref edge to the target and emits
    /// it.
    ///
    /// # Errors
    /// If an error occurs while writing the entry, an error is returned.
    pub fn emit_reference(
        &mut self,
        anchor_vname: &VName,
        target_vname: &VName,
        byte_start: u32,
        byte_end: u32,
    ) -> Result<(), KytheError> {
        self.emit_node(anchor_vname, "/kythe/node/kind", b"anchor".to_vec())?;
        self.emit_node(
            anchor_vname,
            "/kythe/loc/start",
            byte_start.to_string().into_bytes().to_vec(),
        )?;
        self.emit_node(
            anchor_vname,
            "/kythe/loc/end",
            byte_end.to_string().into_bytes().to_vec(),
        )?;
        self.emit_edge(anchor_vname, target_vname, "/kythe/edge/ref")
    }
}
