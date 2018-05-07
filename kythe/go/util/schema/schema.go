/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package schema defines constants used in the Kythe schema.
package schema

import (
	"kythe.io/kythe/go/util/schema/facts"

	"github.com/golang/protobuf/proto"

	scpb "kythe.io/kythe/proto/schema_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Prefix is the label prefix for the Kythe schema.
const Prefix = "/kythe/"

const (
	// AnchorLocFilter is a fact filter for anchor locations.
	AnchorLocFilter = "/kythe/loc/*"

	// SnippetLocFilter is a fact filter for snippet locations.
	SnippetLocFilter = "/kythe/snippet/*"
)

// An Edge represents an edge.
type Edge struct {
	Source, Target *spb.VName
	Kind           string
}

// ToEntry converts e to a kythe.proto.Entry message.
func (e *Edge) ToEntry() *spb.Entry {
	return &spb.Entry{
		Source:   e.Source,
		Target:   e.Target,
		EdgeKind: e.Kind,
		FactName: "/",
	}
}

// Facts represents a collection of key/value facts.
type Facts map[string]string

// A Node represents a collection of facts about a node.
type Node struct {
	VName *spb.VName
	Kind  string
	Facts Facts
}

// AddFact adds the specified fact to n, replacing any previous value for that
// fact that may exist.
func (n *Node) AddFact(name, value string) {
	if n.Facts == nil {
		n.Facts = make(Facts)
	}
	n.Facts[name] = value
}

// ToEntries converts n to a slice of kythe.proto.Entry messages. The result
// will have at least one entry for the node kind. If n contains a text fact
// and does not supply an explicit encoding, the default one is also added.
// The resulting slice is not ordered.
func (n *Node) ToEntries() []*spb.Entry {
	var entries []*spb.Entry
	add := func(key, value string) {
		entries = append(entries, &spb.Entry{
			Source:    n.VName,
			FactName:  key,
			FactValue: []byte(value),
		})
	}

	add(facts.NodeKind, n.Kind) // ensure the node kind exists.
	for name, value := range n.Facts {
		add(name, value)
	}

	// If a text fact was emitted, ensure there is an encoding too.
	if _, hasText := n.Facts[facts.Text]; hasText {
		if _, hasEncoding := n.Facts[facts.TextEncoding]; !hasEncoding {
			add(facts.TextEncoding, facts.DefaultTextEncoding)
		}
	}
	return entries
}

var (
	nodeKinds = make(map[string]scpb.NodeKind)
	subkinds  = make(map[string]scpb.Subkind)
	factNames = make(map[string]scpb.FactName)
	edgeKinds = make(map[string]scpb.EdgeKind)

	nodeKindsRev = make(map[scpb.NodeKind]string)
	subkindsRev  = make(map[scpb.Subkind]string)
	factNamesRev = make(map[scpb.FactName]string)
	edgeKindsRev = make(map[scpb.EdgeKind]string)
)

func init() {
	var index scpb.Index
	if err := proto.UnmarshalText(indexTextPB, &index); err != nil {
		panic(err)
	}
	for _, kinds := range index.NodeKinds {
		for name, enum := range kinds.NodeKind {
			name = kinds.Prefix + name
			nodeKinds[name] = enum
			nodeKindsRev[enum] = name
		}
	}
	for _, kinds := range index.Subkinds {
		for name, enum := range kinds.Subkind {
			name = kinds.Prefix + name
			subkinds[name] = enum
			subkindsRev[enum] = name
		}
	}
	for _, kinds := range index.FactNames {
		for name, enum := range kinds.FactName {
			name = kinds.Prefix + name
			factNames[name] = enum
			factNamesRev[enum] = name
		}
	}
	for _, kinds := range index.EdgeKinds {
		for name, enum := range kinds.EdgeKind {
			name = kinds.Prefix + name
			edgeKinds[name] = enum
			edgeKindsRev[enum] = name
		}
	}
}

// NodeKind returns the schema enum for the given node kind.
func NodeKind(k string) scpb.NodeKind { return nodeKinds[k] }

// EdgeKind returns the schema enum for the given edge kind.
func EdgeKind(k string) scpb.EdgeKind { return edgeKinds[k] }

// FactName returns the schema enum for the given fact name.
func FactName(f string) scpb.FactName { return factNames[f] }

// Subkind returns the schema enum for the given subkind.
func Subkind(k string) scpb.Subkind { return subkinds[k] }

// NodeKindString returns the string representation of the given node kind.
func NodeKindString(k scpb.NodeKind) string { return nodeKindsRev[k] }

// EdgeKindString returns the string representation of the given edge kind.
func EdgeKindString(k scpb.EdgeKind) string { return edgeKindsRev[k] }

// FactNameString returns the string representation of the given fact name.
func FactNameString(f scpb.FactName) string { return factNamesRev[f] }

// SubkindString returns the string representation of the given subkind.
func SubkindString(k scpb.Subkind) string { return subkindsRev[k] }
