// Program mkdata converts a text protobuf containing Kythe schema descriptors
// into a Go source file that can be compiled into the schema package.
package main

import (
	"flag"
	"fmt"
	"go/format"
	"io/ioutil"
	"log"
	"reflect"
	"sort"
	"strings"
	"time"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"

	scpb "kythe.io/kythe/proto/schema_go_proto"
)

var (
	inputPath   = flag.String("input", "", "Path of input data file (textpb)")
	outputPath  = flag.String("output", "", "Path of output source file (Go)")
	packageName = flag.String("package", "schema", "Package name to generate")
)

func main() {
	flag.Parse()
	switch {
	case *inputPath == "":
		log.Fatal("You must provide a data -input path")
	case *outputPath == "":
		log.Fatal("You must provide a source -output path")
	case *packageName == "":
		log.Fatal("You must provide a source -package name")
	}

	// Read and decode the schema constants.
	data, err := ioutil.ReadFile(*inputPath)
	if err != nil {
		log.Fatalf("Reading input data: %v", err)
	}
	var index scpb.Index
	if err := proto.UnmarshalText(string(data), &index); err != nil {
		log.Fatalf("Invalid schema data: %v", err)
	}

	// Generate Go source text.
	src := new(strings.Builder)
	fmt.Fprintf(src, `/*
 * Copyright %s Google Inc. All rights reserved.
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
`, time.Now().Format("2006"))
	fmt.Fprintf(src, "\n\npackage %s\n", *packageName)
	fmt.Fprintf(src, `
// This is a generated file -- do not edit it by hand.
// Input file: %s

`, *inputPath)

	fmt.Fprintln(src, `import scpb "kythe.io/kythe/proto/schema_go_proto"`)
	fmt.Fprintln(src, `var (`)

	fmt.Fprintln(src, "nodeKinds = map[string]scpb.NodeKind{")
	nodeKindsRev := make(map[scpb.NodeKind]string)
	for _, kinds := range index.NodeKinds {
		for _, name := range stringset.FromKeys(kinds.NodeKind).Elements() {
			enum := kinds.NodeKind[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, "%q: %d,\n", fullName, enum)
			nodeKindsRev[enum] = fullName
		}
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nsubkinds = map[string]scpb.Subkind{")
	subkindsRev := make(map[scpb.Subkind]string)
	for _, kinds := range index.Subkinds {
		for _, name := range stringset.FromKeys(kinds.Subkind).Elements() {
			enum := kinds.Subkind[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, "%q: %d,\n", fullName, enum)
			subkindsRev[enum] = fullName
		}
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nfactNames = map[string]scpb.FactName{")
	factNamesRev := make(map[scpb.FactName]string)
	for _, kinds := range index.FactNames {
		for _, name := range stringset.FromKeys(kinds.FactName).Elements() {
			enum := kinds.FactName[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, "%q: %d,\n", fullName, enum)
			factNamesRev[enum] = fullName
		}
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nedgeKinds = map[string]scpb.EdgeKind{")
	edgeKindsRev := make(map[scpb.EdgeKind]string)
	for _, kinds := range index.EdgeKinds {
		for _, name := range stringset.FromKeys(kinds.EdgeKind).Elements() {
			enum := kinds.EdgeKind[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, "%q: %d,\n", fullName, enum)
			edgeKindsRev[enum] = fullName
		}
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nnodeKindsRev = map[scpb.NodeKind]string{")
	for _, kind := range sortedKeys(nodeKindsRev) {
		enum := kind.(scpb.NodeKind)
		name := nodeKindsRev[enum]
		fmt.Fprintf(src, "%d: %q,\n", enum, name)
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nsubkindsRev = map[scpb.Subkind]string{")
	for _, kind := range sortedKeys(subkindsRev) {
		enum := kind.(scpb.Subkind)
		name := subkindsRev[enum]
		fmt.Fprintf(src, "%d: %q,\n", enum, name)
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nfactNamesRev = map[scpb.FactName]string{")
	for _, kind := range sortedKeys(factNamesRev) {
		enum := kind.(scpb.FactName)
		name := factNamesRev[enum]
		fmt.Fprintf(src, "%d: %q,\n", enum, name)
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nedgeKindsRev = map[scpb.EdgeKind]string{")
	for _, kind := range sortedKeys(edgeKindsRev) {
		enum := kind.(scpb.EdgeKind)
		name := edgeKindsRev[enum]
		fmt.Fprintf(src, "%d: %q,\n", enum, name)
	}
	fmt.Fprintln(src, "}")
	fmt.Fprintln(src, ")")

	fmt.Fprintln(src, `
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
`)

	// Format and write out the resulting program.
	text, err := format.Source([]byte(src.String()))
	if err != nil {
		log.Fatalf("Formatting Go source: %v", err)
	}
	if err := ioutil.WriteFile(*outputPath, text, 0644); err != nil {
		log.Fatalf("Writing Go output: %v", err)
	}
}

var u32 = reflect.TypeOf(uint32(0))

// sortedKeys returns a slice of the keys of v having type map[X]V, where X is
// a type convertible to uint32 (e.g., a proto enumeration).  This function
// will panic if v does not have an appropriate concrete type.  The concrete
// type of each returned element remains X (not uint32).
func sortedKeys(v interface{}) []interface{} {
	keys := reflect.ValueOf(v).MapKeys()
	sort.Slice(keys, func(i, j int) bool {
		a := keys[i].Convert(u32).Interface().(uint32)
		b := keys[j].Convert(u32).Interface().(uint32)
		return a < b
	})
	vals := make([]interface{}, len(keys))
	for i, key := range keys {
		vals[i] = key.Interface()
	}
	return vals
}
