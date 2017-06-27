/*
 * Copyright 2017 Google Inc. All rights reserved.
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

// Binary extract_schema extracts a machine-readable representation of the
// Kythe schema from the schema documentation.  Output is written as JSON to
// stdout.
//
// Usage:
//    extractschema -schema kythe/docs/schema/schema.txt
//
package main

import (
	"encoding/json"
	"flag"
	"io/ioutil"
	"log"
	"os"
	"regexp"
	"sort"
	"strings"
)

// Schema represents the schema as a whole.
type Schema struct {
	Nodes []*Node `json:"nodes,omitempty"`
	Edges []*Edge `json:"edges,omitempty"`
}

// findNodeKind returns the *Node representing nodes of the given kind, or nil
// if no such node kind exists in the schema.
func (s Schema) findNodeKind(kind string) *Node {
	for _, node := range s.Nodes {
		if node.Kind == kind {
			return node
		}
	}
	return nil
}

// A Node carries metadata about a single node kind in the schema.
type Node struct {
	Kind        string   `json:"kind"`
	Description string   `json:"description,omitempty"`
	Facts       []*Fact  `json:"facts,omitempty"` // applicable facts
	Edges       []string `json:"edges,omitempty"` // related edge kinds
	Related     []string `json:"rel,omitempty"`   // related node kinds
}

// addEdgeKind adds kind to the set of edge kinds for n, if it is not already
// present.
func (n *Node) addEdgeKind(kind string) {
	if n == nil {
		return
	}
	for _, existing := range n.Edges {
		if existing == kind {
			return
		}
	}
	n.Edges = append(n.Edges, kind)
}

type nodesByKind []*Node

func (b nodesByKind) Len() int           { return len(b) }
func (b nodesByKind) Less(i, j int) bool { return b[i].Kind < b[j].Kind }
func (b nodesByKind) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }

// An Edge carries metadata about a single edge kind in the schema.
type Edge struct {
	Kind        string   `json:"kind"`
	Description string   `json:"description,omitempty"`
	Ordinal     bool     `json:"ordinal,omitempty"`
	Source      []string `json:"source,omitempty"` // source node kinds
	Target      []string `json:"target,omitempty"` // target node kinds
}

type edgesByKind []*Edge

func (b edgesByKind) Len() int           { return len(b) }
func (b edgesByKind) Less(i, j int) bool { return b[i].Kind < b[j].Kind }
func (b edgesByKind) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }

// A Fact carries metadata about a single fact label.
type Fact struct {
	Label       string   `json:"label"`
	Description string   `json:"description,omitempty"`
	Values      []string `json:"values,omitempty"`
}

var (
	schemaFile = flag.String("schema", "", "Schema file path (required)")

	beginSection = regexp.MustCompile(`(?m)^([ \w]+?)$\n--{1,50}$`)
	kindHeader   = regexp.MustCompile(`(?m)^\[\[\w+\]\]\n([^\n]+)\n~+$`)
	mainLabel    = regexp.MustCompile(`(?m)^([- \w]+)::$`)
	subLabel     = regexp.MustCompile(`(?m) +([ \w/]+):::$`)
	kindLink     = regexp.MustCompile(`\b(semantic) nodes\b|\b(anchor)s\b|<<([\w/]+)(?:,\w+)?>>`)
	factLink     = regexp.MustCompile("`([^`]+)`")
)

func main() {
	flag.Parse()
	if *schemaFile == "" {
		log.Fatal("You must provide the path to the --schema file")
	}

	data, err := ioutil.ReadFile(*schemaFile)
	if err != nil {
		log.Fatalf("Reading schema fila: %v", err)
	}

	var schema Schema
	sections := splitOnRegexp(beginSection, string(data))
	if s, ok := sections["node kinds"]; ok {
		schema.Nodes = extractNodeKinds(s)
	}

	sort.Sort(nodesByKind(schema.Nodes))
	if s, ok := sections["edge kinds"]; ok {
		schema.Edges = extractEdgeKinds(s)
	}
	sort.Sort(edgesByKind(schema.Edges))

	// Add the kind of each edge to the edges set of any node mentioned in the
	// source or targets list for that edge.
	for _, edge := range schema.Edges {
		for _, kind := range edge.Source {
			schema.findNodeKind(kind).addEdgeKind(edge.Kind)
		}
		for _, kind := range edge.Target {
			schema.findNodeKind(kind).addEdgeKind(edge.Kind)
		}
	}

	enc := json.NewEncoder(os.Stdout)
	if err := enc.Encode(&schema); err != nil {
		log.Printf("Error encoding schema: %v", err)
	}
}

func extractNodeKinds(s string) []*Node {
	var out []*Node

	for kind, text := range splitOnRegexp(kindHeader, s) {
		labels := splitOnRegexp(mainLabel, text)
		node := &Node{
			Kind:        kind,
			Description: cleanText(labels["brief description"]),
		}
		for name, desc := range splitOnRegexp(subLabel, labels["facts"]) {
			fact := &Fact{Label: name, Description: collapseLines(trimExtra(desc))}
			for _, val := range factLink.FindAllStringSubmatch(fact.Description, -1) {
				fact.Values = append(fact.Values, val[1])
			}
			sort.Strings(fact.Values)
			node.Facts = append(node.Facts, fact)

		}
		for _, target := range kindLink.FindAllStringSubmatch(node.Description, -1) {
			node.Related = append(node.Related, nonempty(target[1:])...)
		}
		for _, target := range kindLink.FindAllStringSubmatch(labels["expected out-edges"], -1) {
			node.Edges = append(node.Edges, nonempty(target[1:])...)
		}
		out = append(out, node)
	}
	return out
}

func extractEdgeKinds(s string) []*Edge {
	var out []*Edge

	for kind, text := range splitOnRegexp(kindHeader, s) {
		labels := splitOnRegexp(mainLabel, text)
		edge := &Edge{
			Kind:        kind,
			Description: cleanText(labels["brief description"]),
		}
		if t := cleanText(labels["ordinals are used"]); t == "always" {
			edge.Ordinal = true
		}
		for _, target := range kindLink.FindAllStringSubmatch(labels["points toward"], -1) {
			edge.Target = append(edge.Target, nonempty(target[1:])...)
		}
		for _, source := range kindLink.FindAllStringSubmatch(labels["points from"], -1) {
			edge.Source = append(edge.Source, nonempty(source[1:])...)
		}
		out = append(out, edge)
	}
	return out
}

// splitOnRegexp partitions s into sections on the given regexp, which must
// define at least one capture group. The contents of the capture group are
// used as the name, and the text between matches becomes the value.
// All names are normalized to lower-case.
func splitOnRegexp(expr *regexp.Regexp, s string) map[string]string {
	out := make(map[string]string)

	prev := ""
	last := 0
	for _, pos := range expr.FindAllStringSubmatchIndex(s, -1) {
		name := strings.ToLower(s[pos[2]:pos[3]])
		if prev != "" {
			out[prev] = s[last:pos[0]]
		}
		prev = name
		last = pos[1]
	}
	if prev != "" {
		out[prev] = s[last:]
	}
	return out
}

// cleanText cleans up s by trimming whitespace and collapsing lines.
func cleanText(s string) string { return collapseLines(trimExtra(s)) }

// trimExtra discards from s anything after the first blank line.
func trimExtra(s string) string {
	if i := strings.Index(s, "\n\n"); i >= 0 {
		return s[:i]
	}
	return s
}

// collapseLines splits s on newlines, trims whitespace from each resulting
// line, discards any blanks, and returns the remainder joined by spaces.
func collapseLines(s string) string {
	var lines []string
	for _, line := range strings.Split(s, "\n") {
		if clean := strings.Trim(line, " *"); clean != "" {
			lines = append(lines, clean)
		}
	}
	return strings.Join(lines, " ")
}

// nonempty filters empty strings from s.
func nonempty(ss []string) (out []string) {
	for _, s := range ss {
		if s != "" {
			out = append(out, s)
		}
	}
	return
}
