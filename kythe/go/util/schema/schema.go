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

	spb "kythe.io/kythe/proto/storage_proto"
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
