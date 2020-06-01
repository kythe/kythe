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

// Package columnar implements the columnar table format for a Kythe xrefs service.
package columnar // import "kythe.io/kythe/go/serving/xrefs/columnar"

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"

	"kythe.io/kythe/go/util/keys"
	"kythe.io/kythe/go/util/kytheuri"

	"google.golang.org/protobuf/proto"

	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xspb "kythe.io/kythe/proto/xref_serving_go_proto"
)

var (
	// DecorationsKeyPrefix is the common key prefix for all Kythe columnar
	// FileDecoration key-value entries.
	DecorationsKeyPrefix, _ = keys.Append(nil, "fd")

	// CrossReferencesKeyPrefix is the common key prefix for all Kythe columnar
	// CrossReferences key-value entries.
	CrossReferencesKeyPrefix, _ = keys.Append(nil, "xr")
)

func init() {
	// Restrict the capacity of the key prefixes to ensure appending to it creates a new array.
	DecorationsKeyPrefix = DecorationsKeyPrefix[:len(DecorationsKeyPrefix):len(DecorationsKeyPrefix)]
	CrossReferencesKeyPrefix = CrossReferencesKeyPrefix[:len(CrossReferencesKeyPrefix):len(CrossReferencesKeyPrefix)]
}

// Columnar file decorations group numbers.
// See: kythe/proto/xref_serving.proto
const (
	columnarDecorationsIndexGroup              = -1 // no group number
	columnarDecorationsTextGroup               = 0
	columnarDecorationsTargetGroup             = 10
	columnarDecorationsTargetOverrideGroup     = 20
	columnarDecorationsTargetNodeGroup         = 30
	columnarDecorationsTargetDefinitionGroup   = 40
	columnarDecorationsDefinitionLocationGroup = 50
	columnarDecorationsOverrideGroup           = 60
	columnarDecorationsDiagnosticGroup         = 70
)

// KV is a single columnar key-value entry.
type KV struct{ Key, Value []byte }

// EncodeDecorationsEntry encodes a columnar FileDecorations entry.
func EncodeDecorationsEntry(keyPrefix []byte, fd *xspb.FileDecorations) (*KV, error) {
	switch e := fd.Entry.(type) {
	case *xspb.FileDecorations_Index_:
		return encodeDecorIndex(keyPrefix, fd.File, e.Index)
	case *xspb.FileDecorations_Text_:
		return encodeDecorText(keyPrefix, fd.File, e.Text)
	case *xspb.FileDecorations_Target_:
		return encodeDecorTarget(keyPrefix, fd.File, e.Target)
	case *xspb.FileDecorations_TargetOverride_:
		return encodeDecorTargetOverride(keyPrefix, fd.File, e.TargetOverride)
	case *xspb.FileDecorations_TargetNode_:
		return encodeDecorTargetNode(keyPrefix, fd.File, e.TargetNode)
	case *xspb.FileDecorations_TargetDefinition_:
		return encodeDecorTargetDefinition(keyPrefix, fd.File, e.TargetDefinition)
	case *xspb.FileDecorations_DefinitionLocation_:
		return encodeDecorDefinitionLocation(keyPrefix, fd.File, e.DefinitionLocation)
	case *xspb.FileDecorations_Override_:
		return encodeDecorOverride(keyPrefix, fd.File, e.Override)
	case *xspb.FileDecorations_Diagnostic_:
		return encodeDecorDiagnostic(keyPrefix, fd.File, e.Diagnostic)
	default:
		return nil, fmt.Errorf("unknown FileDecorations entry: %T", e)
	}
}

func encodeDecorIndex(prefix []byte, file *spb.VName, idx *xspb.FileDecorations_Index) (*KV, error) {
	key, err := keys.Append(prefix, file)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(idx)
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeDecorText(prefix []byte, file *spb.VName, t *xspb.FileDecorations_Text) (*KV, error) {
	key, err := keys.Append(prefix, file, columnarDecorationsTextGroup, t.StartOffset, t.EndOffset)
	if err != nil {
		return nil, err
	}
	// Encode the subset of the value proto not encoded in the key.
	val, err := proto.Marshal(&xspb.FileDecorations_Text{Text: t.Text})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeDecorTarget(prefix []byte, file *spb.VName, t *xspb.FileDecorations_Target) (*KV, error) {
	key, err := keys.Append(prefix, file,
		columnarDecorationsTargetGroup, t.BuildConfig, t.StartOffset, t.EndOffset,
		t.GetGenericKind(), int32(t.GetKytheKind()), t.Target)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&xspb.FileDecorations_Target{})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeDecorTargetOverride(prefix []byte, file *spb.VName, to *xspb.FileDecorations_TargetOverride) (*KV, error) {
	key, err := keys.Append(prefix, file,
		columnarDecorationsTargetOverrideGroup, to.Overridden, int32(to.Kind), to.Overriding)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&xspb.FileDecorations_TargetOverride{})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}
func encodeDecorTargetNode(prefix []byte, file *spb.VName, tn *xspb.FileDecorations_TargetNode) (*KV, error) {
	key, err := keys.Append(prefix, file, columnarDecorationsTargetNodeGroup, tn.Node.Source)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(tn)
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}
func encodeDecorTargetDefinition(prefix []byte, file *spb.VName, td *xspb.FileDecorations_TargetDefinition) (*KV, error) {
	key, err := keys.Append(prefix, file, columnarDecorationsTargetDefinitionGroup, td.Target)
	if err != nil {
		return nil, err
	}
	// Encode the subset of the value proto not encoded in the key.
	val, err := proto.Marshal(&xspb.FileDecorations_TargetDefinition{Definition: td.Definition})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}
func encodeDecorDefinitionLocation(prefix []byte, file *spb.VName, def *xspb.FileDecorations_DefinitionLocation) (*KV, error) {
	// TODO(schroederc): use VNames throughout pipeline
	defVName, err := kytheuri.ToVName(def.Location.Ticket)
	if err != nil {
		return nil, err
	}
	key, err := keys.Append(prefix, file, columnarDecorationsDefinitionLocationGroup, defVName)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(def)
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}
func encodeDecorOverride(prefix []byte, file *spb.VName, o *xspb.FileDecorations_Override) (*KV, error) {
	key, err := keys.Append(prefix, file, columnarDecorationsOverrideGroup, o.Override)
	if err != nil {
		return nil, err
	}
	// Encode the subset of the value proto not encoded in the key.
	val, err := proto.Marshal(&xspb.FileDecorations_Override{MarkedSource: o.MarkedSource})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}
func encodeDecorDiagnostic(prefix []byte, file *spb.VName, d *xspb.FileDecorations_Diagnostic) (*KV, error) {
	val, err := proto.Marshal(d)
	if err != nil {
		return nil, err
	}
	h := sha256.Sum224(val)
	key, err := keys.Append(prefix, file, columnarDecorationsDiagnosticGroup, d.Diagnostic.Span.GetStart().GetByteOffset(), d.Diagnostic.Span.GetEnd().GetByteOffset(), hex.EncodeToString(h[:]))
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

// DecodeDecorationsEntry decodes a columnar FileDecorations entry.
func DecodeDecorationsEntry(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	kind := columnarDecorationsIndexGroup
	if key != "" {
		var err error
		key, err = keys.Parse(key, &kind)
		if err != nil {
			return nil, fmt.Errorf("invalid FileDecorations group kind: %v", err)
		}
	}
	switch kind {
	case columnarDecorationsIndexGroup:
		return decodeDecorIndex(file, key, val)
	case columnarDecorationsTextGroup:
		return decodeDecorText(file, key, val)
	case columnarDecorationsTargetGroup:
		return decodeDecorTarget(file, key, val)
	case columnarDecorationsTargetOverrideGroup:
		return decodeDecorTargetOverride(file, key, val)
	case columnarDecorationsTargetNodeGroup:
		return decodeDecorTargetNode(file, key, val)
	case columnarDecorationsTargetDefinitionGroup:
		return decodeDecorTargetDefinition(file, key, val)
	case columnarDecorationsDefinitionLocationGroup:
		return decodeDecorDefinitionLocation(file, key, val)
	case columnarDecorationsOverrideGroup:
		return decodeDecorOverride(file, key, val)
	case columnarDecorationsDiagnosticGroup:
		return decodeDecorDiagnostic(file, key, val)
	default:
		return nil, fmt.Errorf("unknown group kind: %d", kind)
	}
}

func decodeDecorIndex(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var idx xspb.FileDecorations_Index
	if err := proto.Unmarshal(val, &idx); err != nil {
		return nil, err
	}
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_Index_{&idx},
	}, nil
}

func decodeDecorText(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var text xspb.FileDecorations_Text
	if err := proto.Unmarshal(val, &text); err != nil {
		return nil, err
	}
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_Text_{&text},
	}, nil
}

func decodeDecorTarget(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	target := &xspb.FileDecorations_Target{Target: &spb.VName{}}
	var (
		kytheKindNum int32
		genericKind  string
	)
	key, err := keys.Parse(key,
		&target.BuildConfig, &target.StartOffset, &target.EndOffset,
		&genericKind, &kytheKindNum, target.Target)
	if err != nil {
		return nil, err
	}

	if genericKind != "" {
		target.Kind = &xspb.FileDecorations_Target_GenericKind{genericKind}
	} else {
		target.Kind = &xspb.FileDecorations_Target_KytheKind{scpb.EdgeKind(kytheKindNum)}
	}
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_Target_{target},
	}, nil
}

func decodeDecorTargetOverride(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var overridden, overriding spb.VName
	var kind int32
	key, err := keys.Parse(key, &overridden, &kind, &overriding)
	if err != nil {
		return nil, err
	} else if key != "" {
		return nil, fmt.Errorf("unexpected TargetOverride key suffix: %q", key)
	}
	return &xspb.FileDecorations{
		File: file,
		Entry: &xspb.FileDecorations_TargetOverride_{&xspb.FileDecorations_TargetOverride{
			Kind:       xspb.FileDecorations_TargetOverride_Kind(kind),
			Overridden: &overridden,
			Overriding: &overriding,
		}},
	}, nil
}

func decodeDecorTargetNode(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var tn xspb.FileDecorations_TargetNode
	if err := proto.Unmarshal(val, &tn); err != nil {
		return nil, err
	}
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_TargetNode_{&tn},
	}, nil
}

func decodeDecorTargetDefinition(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var target spb.VName
	key, err := keys.Parse(key, &target)
	if err != nil {
		return nil, err
	} else if key != "" {
		return nil, fmt.Errorf("unexpected TargetDefinition key suffix: %q", key)
	}
	var def xspb.FileDecorations_TargetDefinition
	if err := proto.Unmarshal(val, &def); err != nil {
		return nil, err
	}
	def.Target = &target
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_TargetDefinition_{&def},
	}, nil
}

func decodeDecorDefinitionLocation(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var loc xspb.FileDecorations_DefinitionLocation
	if err := proto.Unmarshal(val, &loc); err != nil {
		return nil, err
	}
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_DefinitionLocation_{&loc},
	}, nil
}

func decodeDecorOverride(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var target spb.VName
	key, err := keys.Parse(key, &target)
	if err != nil {
		return nil, err
	} else if key != "" {
		return nil, fmt.Errorf("unexpected Override key suffix: %q", key)
	}
	var o xspb.FileDecorations_Override
	if err := proto.Unmarshal(val, &o); err != nil {
		return nil, err
	}
	o.Override = &target
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_Override_{&o},
	}, nil
}

func decodeDecorDiagnostic(file *spb.VName, key string, val []byte) (*xspb.FileDecorations, error) {
	var o xspb.FileDecorations_Diagnostic
	if err := proto.Unmarshal(val, &o); err != nil {
		return nil, err
	}
	return &xspb.FileDecorations{
		File:  file,
		Entry: &xspb.FileDecorations_Diagnostic_{&o},
	}, nil
}

// Columnar file decorations group numbers.
// See: kythe/proto/xref_serving.proto
const (
	columnarXRefsIndexGroup          = -1 // no group number
	columnarXRefsReferenceGroup      = 0
	columnarXRefsRelationGroup       = 10
	columnarXRefsCallerGroup         = 20
	columnarXRefsRelatedNodeGroup    = 30
	columnarXRefsNodeDefinitionGroup = 40
)

// EncodeCrossReferencesEntry encodes a columnar CrossReferences entry.
func EncodeCrossReferencesEntry(keyPrefix []byte, xr *xspb.CrossReferences) (*KV, error) {
	switch e := xr.Entry.(type) {
	case *xspb.CrossReferences_Index_:
		return encodeXRefIndex(keyPrefix, xr.Source, e.Index)
	case *xspb.CrossReferences_Reference_:
		return encodeXRefReference(keyPrefix, xr.Source, e.Reference)
	case *xspb.CrossReferences_Relation_:
		return encodeXRefRelation(keyPrefix, xr.Source, e.Relation)
	case *xspb.CrossReferences_Caller_:
		return encodeXRefCaller(keyPrefix, xr.Source, e.Caller)
	case *xspb.CrossReferences_Callsite_:
		return encodeXRefCallsite(keyPrefix, xr.Source, e.Callsite)
	case *xspb.CrossReferences_RelatedNode_:
		return encodeXRefRelatedNode(keyPrefix, xr.Source, e.RelatedNode)
	case *xspb.CrossReferences_NodeDefinition_:
		return encodeXRefNodeDefinition(keyPrefix, xr.Source, e.NodeDefinition)
	default:
		return nil, fmt.Errorf("unknown CrossReferences entry: %T", e)
	}
}

func encodeXRefIndex(prefix []byte, src *spb.VName, idx *xspb.CrossReferences_Index) (*KV, error) {
	key, err := keys.Append(prefix, src)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(idx)
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeXRefReference(prefix []byte, src *spb.VName, r *xspb.CrossReferences_Reference) (*KV, error) {
	uri, err := kytheuri.Parse(r.Location.Ticket)
	if err != nil {
		return nil, err
	}
	file := &spb.VName{
		Corpus: uri.Corpus,
		Root:   uri.Root,
		Path:   uri.Path,
	}
	key, err := keys.Append(prefix, src, columnarXRefsReferenceGroup,
		r.GetGenericKind(), int32(r.GetKytheKind()), file,
		r.Location.Span.GetStart().GetByteOffset(), r.Location.Span.GetEnd().GetByteOffset())
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&xspb.CrossReferences_Reference{Location: r.Location})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeXRefRelation(prefix []byte, src *spb.VName, r *xspb.CrossReferences_Relation) (*KV, error) {
	key, err := keys.Append(prefix, src, columnarXRefsRelationGroup,
		r.GetGenericKind(), int32(r.GetKytheKind()), r.Ordinal, r.Reverse, r.Node)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&xspb.CrossReferences_Relation{})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeXRefCaller(prefix []byte, src *spb.VName, c *xspb.CrossReferences_Caller) (*KV, error) {
	key, err := keys.Append(prefix, src, columnarXRefsCallerGroup, c.Caller)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&xspb.CrossReferences_Caller{
		Location:     c.Location,
		MarkedSource: c.MarkedSource,
	})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeXRefCallsite(prefix []byte, src *spb.VName, c *xspb.CrossReferences_Callsite) (*KV, error) {
	uri, err := kytheuri.Parse(c.Location.Ticket)
	if err != nil {
		return nil, err
	}
	file := &spb.VName{
		Corpus: uri.Corpus,
		Root:   uri.Root,
		Path:   uri.Path,
	}
	key, err := keys.Append(prefix, src, columnarXRefsCallerGroup,
		c.Caller, int32(c.Kind), file,
		c.Location.Span.GetStart().GetByteOffset(), c.Location.Span.GetEnd().GetByteOffset())
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&xspb.CrossReferences_Callsite{
		Location: c.Location,
	})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeXRefRelatedNode(prefix []byte, src *spb.VName, n *xspb.CrossReferences_RelatedNode) (*KV, error) {
	key, err := keys.Append(prefix, src, columnarXRefsRelatedNodeGroup, n.Node.Source)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(n)
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeXRefNodeDefinition(prefix []byte, src *spb.VName, def *xspb.CrossReferences_NodeDefinition) (*KV, error) {
	key, err := keys.Append(prefix, src, columnarXRefsNodeDefinitionGroup, def.Node)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&xspb.CrossReferences_NodeDefinition{Location: def.Location})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

// DecodeCrossReferencesEntry decodes a columnar CrossReferences entry.
func DecodeCrossReferencesEntry(src *spb.VName, key string, val []byte) (*xspb.CrossReferences, error) {
	kind := columnarXRefsIndexGroup
	if key != "" {
		var err error
		key, err = keys.Parse(key, &kind)
		if err != nil {
			return nil, fmt.Errorf("invalid CrossReferences group kind: %v", err)
		}
	}
	switch kind {
	case columnarXRefsIndexGroup:
		return decodeXRefIndex(src, key, val)
	case columnarXRefsReferenceGroup:
		return decodeXRefReference(src, key, val)
	case columnarXRefsRelationGroup:
		return decodeXRefRelation(src, key, val)
	case columnarXRefsCallerGroup:
		return decodeXRefCallerOrCallsite(src, key, val)
	case columnarXRefsRelatedNodeGroup:
		return decodeXRefRelatedNode(src, key, val)
	case columnarXRefsNodeDefinitionGroup:
		return decodeXRefNodeDefinition(src, key, val)
	default:
		return nil, fmt.Errorf("unknown group kind: %d", kind)
	}
}

func decodeXRefIndex(src *spb.VName, key string, val []byte) (*xspb.CrossReferences, error) {
	var idx xspb.CrossReferences_Index
	if err := proto.Unmarshal(val, &idx); err != nil {
		return nil, err
	}
	return &xspb.CrossReferences{
		Source: src,
		Entry:  &xspb.CrossReferences_Index_{&idx},
	}, nil
}

func decodeXRefReference(src *spb.VName, key string, val []byte) (*xspb.CrossReferences, error) {
	var (
		genericKind string
		kytheKind   int32
	)
	key, err := keys.Parse(key, &genericKind, &kytheKind)
	if err != nil {
		return nil, err
	}
	var ref xspb.CrossReferences_Reference
	if err := proto.Unmarshal(val, &ref); err != nil {
		return nil, err
	}
	if genericKind != "" {
		ref.Kind = &xspb.CrossReferences_Reference_GenericKind{genericKind}
	} else {
		ref.Kind = &xspb.CrossReferences_Reference_KytheKind{scpb.EdgeKind(kytheKind)}
	}
	return &xspb.CrossReferences{
		Source: src,
		Entry:  &xspb.CrossReferences_Reference_{&ref},
	}, nil
}

func decodeXRefRelation(src *spb.VName, key string, val []byte) (*xspb.CrossReferences, error) {
	var (
		genericKind string
		kytheKind   int32
		ordinal     int32
		reverse     bool
		node        spb.VName
	)
	key, err := keys.Parse(key, &genericKind, &kytheKind, &ordinal, &reverse, &node)
	if err != nil {
		return nil, err
	}
	var r xspb.CrossReferences_Relation
	if err := proto.Unmarshal(val, &r); err != nil {
		return nil, err
	}
	r.Reverse = reverse
	r.Ordinal = ordinal
	r.Node = &node
	if genericKind != "" {
		r.Kind = &xspb.CrossReferences_Relation_GenericKind{genericKind}
	} else {
		r.Kind = &xspb.CrossReferences_Relation_KytheKind{scpb.EdgeKind(kytheKind)}
	}
	return &xspb.CrossReferences{
		Source: src,
		Entry:  &xspb.CrossReferences_Relation_{&r},
	}, nil
}

func decodeXRefCallerOrCallsite(src *spb.VName, key string, val []byte) (*xspb.CrossReferences, error) {
	var caller spb.VName
	key, err := keys.Parse(key, &caller)
	if err != nil {
		return nil, err
	}

	if key == "" {
		var c xspb.CrossReferences_Caller
		if err := proto.Unmarshal(val, &c); err != nil {
			return nil, err
		}
		c.Caller = &caller
		return &xspb.CrossReferences{
			Source: src,
			Entry:  &xspb.CrossReferences_Caller_{&c},
		}, nil
	}

	var kind int32
	key, err = keys.Parse(key, &kind)
	if err != nil {
		return nil, err
	}

	var c xspb.CrossReferences_Callsite
	if err := proto.Unmarshal(val, &c); err != nil {
		return nil, err
	}
	c.Caller = &caller
	c.Kind = xspb.CrossReferences_Callsite_Kind(kind)
	return &xspb.CrossReferences{
		Source: src,
		Entry:  &xspb.CrossReferences_Callsite_{&c},
	}, nil
}

func decodeXRefRelatedNode(src *spb.VName, key string, val []byte) (*xspb.CrossReferences, error) {
	var rn xspb.CrossReferences_RelatedNode
	if err := proto.Unmarshal(val, &rn); err != nil {
		return nil, err
	}
	return &xspb.CrossReferences{
		Source: src,
		Entry:  &xspb.CrossReferences_RelatedNode_{&rn},
	}, nil
}

func decodeXRefNodeDefinition(src *spb.VName, key string, val []byte) (*xspb.CrossReferences, error) {
	var node spb.VName
	key, err := keys.Parse(key, &node)
	if err != nil {
		return nil, err
	}
	if len(key) != 0 {
		return nil, fmt.Errorf("unexpected trailing key: %q", key)
	}
	var def xspb.CrossReferences_NodeDefinition
	if err := proto.Unmarshal(val, &def); err != nil {
		return nil, err
	}
	def.Node = &node
	return &xspb.CrossReferences{
		Source: src,
		Entry:  &xspb.CrossReferences_NodeDefinition_{&def},
	}, nil
}
