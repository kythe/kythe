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

use crate::array_writer::ArrayWriter;

extern crate kythe_rust_indexer;
use kythe_rust_indexer::error::KytheError;
use kythe_rust_indexer::indexer::entries::EntryEmitter;
use storage_rust_proto::*;

// This test checks that the emit_fact function works properly
#[test]
fn nodes_properly_emitted() -> Result<(), KytheError> {
    // Create an emitter that writes to the ArrayWriter. This makes it easier to
    // look at the emitted Entry protobufs.
    let mut array_writer = ArrayWriter::new();
    let mut emitter = EntryEmitter::new(&mut array_writer);

    // Create a test VName
    let mut vname = VName::new();
    vname.set_signature("test_signature".to_string());

    emitter.emit_fact(&vname, "/kythe/node/kind", b"file".to_vec())?;

    // Ensure that there is one emitted Entry and that the values match what we
    // expect
    let entries = array_writer.as_ref();
    assert_eq!(entries.len(), 1);

    let entry = entries.get(0).unwrap();
    assert_eq!(*entry.get_source(), vname);
    assert_eq!(entry.get_fact_name(), "/kythe/node/kind");
    assert_eq!(entry.get_fact_value(), b"file".to_vec());

    Ok(())
}

// This test checks that the emit_edge function works properly
#[test]
fn edges_properly_emitted() -> Result<(), KytheError> {
    // Create an emitter that writes to the ArrayWriter. This makes it easier to
    // look at the emitted Entry protobufs.
    let mut array_writer = ArrayWriter::new();
    let mut emitter = EntryEmitter::new(&mut array_writer);

    // Create source and target VNames for the test
    let mut source_vname = VName::new();
    source_vname.set_signature("test_signature_source".to_string());
    let mut target_vname = VName::new();
    target_vname.set_signature("test_signature_target".to_string());

    emitter.emit_edge(&source_vname, &target_vname, "/kythe/edge/documents")?;

    // Check that there is one emitted Entry and that the values match what we
    // expect
    let entries = array_writer.as_ref();
    assert_eq!(entries.len(), 1);

    let entry = entries.get(0).unwrap();
    assert_eq!(*entry.get_source(), source_vname);
    assert_eq!(*entry.get_target(), target_vname);
    assert_eq!(entry.get_fact_name(), "/");
    assert_eq!(entry.get_edge_kind(), "/kythe/edge/documents");

    Ok(())
}

// This test checks that the emit_anchor function works properly
#[test]
fn anchors_properly_emitted() -> Result<(), KytheError> {
    // Create an emitter that writes to the ArrayWriter. This makes it easier to
    // look at the emitted Entry protobufs.
    let mut array_writer = ArrayWriter::new();
    let mut emitter = EntryEmitter::new(&mut array_writer);

    // Create anchor and target VNames for the test
    let mut anchor_vname = VName::new();
    anchor_vname.set_signature("test_signature_anchor".to_string());
    let mut target_vname = VName::new();
    target_vname.set_signature("test_signature_target".to_string());

    emitter.emit_anchor(&anchor_vname, &target_vname, 0u32, 1u32)?;

    // There should be 4 Entry protobufs
    let entries = array_writer.as_ref();
    assert_eq!(entries.len(), 4);

    // The first Entry defines the anchor
    let entry0 = entries.get(0).unwrap();
    assert_eq!(*entry0.get_source(), anchor_vname);
    assert_eq!(entry0.get_fact_name(), "/kythe/node/kind");
    assert_eq!(entry0.get_fact_value(), b"anchor".to_vec());

    // The second and third define the start/end of the anchor
    let entry1 = entries.get(1).unwrap();
    assert_eq!(*entry1.get_source(), anchor_vname);
    assert_eq!(entry1.get_fact_name(), "/kythe/loc/start");
    assert_eq!(entry1.get_fact_value(), b"0".to_vec());

    let entry2 = entries.get(2).unwrap();
    assert_eq!(*entry2.get_source(), anchor_vname);
    assert_eq!(entry2.get_fact_name(), "/kythe/loc/end");
    assert_eq!(entry2.get_fact_value(), b"1".to_vec());

    // The last Entry is the defines/binding edge between the anchor and the target
    let entry3 = entries.get(3).unwrap();
    assert_eq!(*entry3.get_source(), anchor_vname);
    assert_eq!(*entry3.get_target(), target_vname);
    assert_eq!(entry3.get_fact_name(), "/");
    assert_eq!(entry3.get_edge_kind(), "/kythe/edge/defines/binding");

    Ok(())
}
