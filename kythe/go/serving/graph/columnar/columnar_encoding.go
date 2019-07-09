/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this src except in compliance with the License.
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
package columnar // import "kythe.io/kythe/go/serving/graph/columnar"

import (
	"fmt"

	"kythe.io/kythe/go/util/keys"

	"github.com/golang/protobuf/proto"

	gspb "kythe.io/kythe/proto/graph_serving_go_proto"
	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var (
	// EdgesKeyPrefix is the common key prefix for all Kythe columnar Edges
	// key-value entries.
	EdgesKeyPrefix, _ = keys.Append(nil, "eg")
)

func init() {
	// Restrict the capacity of the key prefix to ensure appending to it creates a new array.
	EdgesKeyPrefix = EdgesKeyPrefix[:len(EdgesKeyPrefix):len(EdgesKeyPrefix)]
}

// Columnar edges group numbers.
// See: kythe/proto/graph_serving.proto
const (
	columnarEdgesIndexGroup  = -1 // no group number
	columnarEdgesEdgeGroup   = 10
	columnarEdgesTargetGroup = 20
)

// KV is a single columnar key-value entry.
type KV struct{ Key, Value []byte }

// EncodeEdgesEntry encodes a columnar Edges entry.
func EncodeEdgesEntry(keyPrefix []byte, eg *gspb.Edges) (*KV, error) {
	switch e := eg.Entry.(type) {
	case *gspb.Edges_Index_:
		return encodeEdgesIndex(keyPrefix, eg.Source, e.Index)
	case *gspb.Edges_Edge_:
		return encodeEdge(keyPrefix, eg.Source, e.Edge)
	case *gspb.Edges_Target_:
		return encodeEdgesTarget(keyPrefix, eg.Source, e.Target)
	default:
		return nil, fmt.Errorf("unknown Edges entry: %T", e)
	}
}

func encodeEdgesIndex(prefix []byte, src *spb.VName, idx *gspb.Edges_Index) (*KV, error) {
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

func encodeEdge(prefix []byte, src *spb.VName, e *gspb.Edges_Edge) (*KV, error) {
	key, err := keys.Append(prefix, src, columnarEdgesEdgeGroup,
		e.GetGenericKind(), int32(e.GetKytheKind()), e.Ordinal, e.Reverse, e.Target)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(&gspb.Edges_Edge{})
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

func encodeEdgesTarget(prefix []byte, src *spb.VName, t *gspb.Edges_Target) (*KV, error) {
	key, err := keys.Append(prefix, src, columnarEdgesTargetGroup, t.Node.Source)
	if err != nil {
		return nil, err
	}
	val, err := proto.Marshal(t)
	if err != nil {
		return nil, err
	}
	return &KV{key, val}, nil
}

// DecodeEdgesEntry decodes a columnar Edges entry.
func DecodeEdgesEntry(src *spb.VName, key string, val []byte) (*gspb.Edges, error) {
	kind := columnarEdgesIndexGroup
	if key != "" {
		var err error
		key, err = keys.Parse(key, &kind)
		if err != nil {
			return nil, fmt.Errorf("invalid Edges group kind: %v", err)
		}
	}
	switch kind {
	case columnarEdgesIndexGroup:
		return decodeEdgesIndex(src, key, val)
	case columnarEdgesEdgeGroup:
		return decodeEdge(src, key, val)
	case columnarEdgesTargetGroup:
		return decodeEdgesTarget(src, key, val)
	default:
		return nil, fmt.Errorf("unknown group kind: %d", kind)
	}
}

func decodeEdgesIndex(src *spb.VName, key string, val []byte) (*gspb.Edges, error) {
	var idx gspb.Edges_Index
	if err := proto.Unmarshal(val, &idx); err != nil {
		return nil, err
	}
	return &gspb.Edges{
		Source: src,
		Entry:  &gspb.Edges_Index_{&idx},
	}, nil
}

func decodeEdge(src *spb.VName, key string, val []byte) (*gspb.Edges, error) {
	var (
		genericKind string
		kytheKind   int32
		ordinal     int32
		reverse     bool
		target      spb.VName
	)
	key, err := keys.Parse(key, &genericKind, &kytheKind, &ordinal, &reverse, &target)
	if err != nil {
		return nil, err
	}
	var e gspb.Edges_Edge
	if err := proto.Unmarshal(val, &e); err != nil {
		return nil, err
	}
	e.Reverse = reverse
	e.Ordinal = ordinal
	e.Target = &target
	if genericKind != "" {
		e.Kind = &gspb.Edges_Edge_GenericKind{genericKind}
	} else {
		e.Kind = &gspb.Edges_Edge_KytheKind{scpb.EdgeKind(kytheKind)}
	}
	return &gspb.Edges{
		Source: src,
		Entry:  &gspb.Edges_Edge_{&e},
	}, nil
}

func decodeEdgesTarget(src *spb.VName, key string, val []byte) (*gspb.Edges, error) {
	var tgt gspb.Edges_Target
	if err := proto.Unmarshal(val, &tgt); err != nil {
		return nil, err
	}
	return &gspb.Edges{
		Source: src,
		Entry:  &gspb.Edges_Target_{&tgt},
	}, nil
}
