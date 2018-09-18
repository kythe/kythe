/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package pipeline

import (
	"fmt"
	"reflect"

	"kythe.io/kythe/go/serving/xrefs/columnar"
	"kythe.io/kythe/go/util/kytheuri"

	"github.com/apache/beam/sdks/go/pkg/beam"

	ppb "kythe.io/kythe/proto/pipeline_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xspb "kythe.io/kythe/proto/xref_serving_go_proto"
)

func init() {
	beam.RegisterFunction(encodeCrossRef)
	beam.RegisterFunction(encodeDecorPiece)
	beam.RegisterFunction(refToCrossRef)
	beam.RegisterFunction(nodeToCrossRef)
	beam.RegisterType(reflect.TypeOf((*xspb.CrossReferences)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*xspb.FileDecorations)(nil)).Elem())
}

func encodeCrossRef(xr *xspb.CrossReferences, emit func([]byte, []byte)) error {
	kv, err := columnar.EncodeCrossReferencesEntry(columnar.CrossReferencesKeyPrefix, xr)
	if err != nil {
		return err
	}
	emit(kv.Key, kv.Value)
	return nil
}

func refToCrossRef(r *ppb.Reference) *xspb.CrossReferences {
	ref := &xspb.CrossReferences_Reference{Location: r.Anchor}
	if k := r.GetGenericKind(); k != "" {
		ref.Kind = &xspb.CrossReferences_Reference_GenericKind{k}
	} else {
		ref.Kind = &xspb.CrossReferences_Reference_KytheKind{r.GetKytheKind()}
	}
	return &xspb.CrossReferences{
		Source: r.Source,
		Entry:  &xspb.CrossReferences_Reference_{ref},
	}
}

func nodeToCrossRef(n *scpb.Node) *xspb.CrossReferences {
	return &xspb.CrossReferences{
		Source: n.Source,
		Entry: &xspb.CrossReferences_Index_{&xspb.CrossReferences_Index{
			Node: n,
		}},
	}
}

func encodeDecorPiece(file *spb.VName, p *ppb.DecorationPiece, emit func([]byte, []byte)) error {
	switch p := p.Piece.(type) {
	case *ppb.DecorationPiece_File:
		return encodeDecorFile(file, p.File, emit)
	case *ppb.DecorationPiece_Reference:
		return encodeDecorRef(file, p.Reference, emit)
	case *ppb.DecorationPiece_Node:
		return encodeDecorNode(file, p.Node, emit)
	case *ppb.DecorationPiece_Definition_:
		return encodeDecorDef(file, p.Definition, emit)
	default:
		// TODO(schroederc): add diagnostics/overrides
		return fmt.Errorf("unknown DecorationPiece: %T", p)
	}
}

func encodeDecorFile(file *spb.VName, f *srvpb.File, emit func([]byte, []byte)) error {
	// Emit FileDecorations Index
	e, err := columnar.EncodeDecorationsEntry(columnar.DecorationsKeyPrefix, &xspb.FileDecorations{
		File: file,
		Entry: &xspb.FileDecorations_Index_{&xspb.FileDecorations_Index{
			TextEncoding: f.Encoding,
		}},
	})
	if err != nil {
		return err
	}
	emit(e.Key, e.Value)

	// Encode file contents as single entry
	// TODO(schroederc): chunk large file contents
	e, err = columnar.EncodeDecorationsEntry(columnar.DecorationsKeyPrefix, &xspb.FileDecorations{
		File: file,
		Entry: &xspb.FileDecorations_Text_{&xspb.FileDecorations_Text{
			StartOffset: 0,
			EndOffset:   int32(len(f.Text)),
			Text:        f.Text,
		}},
	})
	if err != nil {
		return err
	}
	emit(e.Key, e.Value)
	return nil
}

func encodeDecorRef(file *spb.VName, ref *ppb.Reference, emit func([]byte, []byte)) error {
	target := &xspb.FileDecorations_Target{
		StartOffset: ref.Anchor.Span.Start.ByteOffset,
		EndOffset:   ref.Anchor.Span.End.ByteOffset,
		Target:      ref.Source,
	}
	if k := ref.GetGenericKind(); k != "" {
		target.Kind = &xspb.FileDecorations_Target_GenericKind{k}
	} else {
		target.Kind = &xspb.FileDecorations_Target_KytheKind{ref.GetKytheKind()}
	}
	e, err := columnar.EncodeDecorationsEntry(columnar.DecorationsKeyPrefix, &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_Target_{target},
	})
	if err != nil {
		return err
	}
	emit(e.Key, e.Value)
	return nil
}

func encodeDecorNode(file *spb.VName, node *scpb.Node, emit func([]byte, []byte)) error {
	e, err := columnar.EncodeDecorationsEntry(columnar.DecorationsKeyPrefix, &xspb.FileDecorations{
		File: file,
		Entry: &xspb.FileDecorations_TargetNode_{&xspb.FileDecorations_TargetNode{
			Node: node,
		}},
	})
	if err != nil {
		return err
	}
	emit(e.Key, e.Value)
	return nil
}

func encodeDecorDef(file *spb.VName, def *ppb.DecorationPiece_Definition, emit func([]byte, []byte)) error {
	// TODO(schroederc): use VNames throughout pipeline
	defVName, err := kytheuri.ToVName(def.Definition.Ticket)
	if err != nil {
		return err
	}
	e, err := columnar.EncodeDecorationsEntry(columnar.DecorationsKeyPrefix, &xspb.FileDecorations{
		File: file,
		Entry: &xspb.FileDecorations_TargetDefinition_{&xspb.FileDecorations_TargetDefinition{
			Target:     def.Node,
			Definition: defVName,
		}},
	})
	if err != nil {
		return err
	}
	emit(e.Key, e.Value)

	e, err = columnar.EncodeDecorationsEntry(columnar.DecorationsKeyPrefix, &xspb.FileDecorations{
		File: file,
		Entry: &xspb.FileDecorations_DefinitionLocation_{&xspb.FileDecorations_DefinitionLocation{
			Location: def.Definition,
		}},
	})
	if err != nil {
		return err
	}
	emit(e.Key, e.Value)
	return nil
}
