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

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"

	scpb "kythe.io/kythe/proto/schema_go_proto"
)

var (
	language    = flag.String("language", "", "Generated target language (supported: go java)")
	inputPath   = flag.String("input", "", "Path of input data file (textpb)")
	outputPath  = flag.String("output", "", "Path of output source file")
	packageName = flag.String("package", "", "Package name to generate")
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

	switch *language {
	case "go":
		generateGo(&index)
	case "java":
		generateJava(&index)
	default:
		log.Fatalf("You must provide a supported generated language (go or java): found %q", *language)
	}
}

const copyrightHeader = `/*
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
 */`

func generateGo(index *scpb.Index) {
	src := new(strings.Builder)
	fmt.Fprintln(src, copyrightHeader)
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

	fmt.Fprint(src, `
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

func generateJava(index *scpb.Index) {
	src := new(strings.Builder)
	fmt.Fprintln(src, copyrightHeader)
	fmt.Fprintln(src, "\n// This is a generated file -- do not edit it by hand.")
	fmt.Fprintf(src, "// Input file: %s\n\n", *inputPath)
	fmt.Fprintf(src, "package %s;\n", *packageName)

	fmt.Fprintf(src, "import com.google.common.collect.ImmutableBiMap;\n")
	fmt.Fprintf(src, "import com.google.devtools.kythe.proto.Schema.EdgeKind;\n")
	fmt.Fprintf(src, "import com.google.devtools.kythe.proto.Schema.FactName;\n")
	fmt.Fprintf(src, "import com.google.devtools.kythe.proto.Schema.NodeKind;\n")
	fmt.Fprintf(src, "import com.google.devtools.kythe.proto.Schema.Subkind;\n")

	fmt.Fprintln(src, "/** Utility for handling schema strings and their corresponding protocol buffer enums. */")
	fmt.Fprintln(src, "public final class Schema {")
	fmt.Fprintln(src, "private Schema() {}")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, NodeKind> NODE_KINDS = ImmutableBiMap.<String, NodeKind>builder()")
	for _, kinds := range index.NodeKinds {
		for _, name := range stringset.FromKeys(kinds.NodeKind).Elements() {
			enum := kinds.NodeKind[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, ".put(%q, NodeKind.%s)\n", fullName, enum)
		}
	}
	fmt.Fprintln(src, ".build();\n")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, Subkind> SUBKINDS = ImmutableBiMap.<String, Subkind>builder()")
	for _, kinds := range index.Subkinds {
		for _, name := range stringset.FromKeys(kinds.Subkind).Elements() {
			enum := kinds.Subkind[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, ".put(%q, Subkind.%s)\n", fullName, enum)
		}
	}
	fmt.Fprintln(src, ".build();\n")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, EdgeKind> EDGE_KINDS = ImmutableBiMap.<String, EdgeKind>builder()")
	for _, kinds := range index.EdgeKinds {
		for _, name := range stringset.FromKeys(kinds.EdgeKind).Elements() {
			enum := kinds.EdgeKind[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, ".put(%q, EdgeKind.%s)\n", fullName, enum)
		}
	}
	fmt.Fprintln(src, ".build();\n")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, FactName> FACT_NAMES = ImmutableBiMap.<String, FactName>builder()")
	for _, kinds := range index.FactNames {
		for _, name := range stringset.FromKeys(kinds.FactName).Elements() {
			enum := kinds.FactName[name]
			fullName := kinds.Prefix + name
			fmt.Fprintf(src, ".put(%q, FactName.%s)\n", fullName, enum)
		}
	}
	fmt.Fprintln(src, ".build();\n")

	fmt.Fprintln(src, "/** Returns the schema {@link NodeKind} for the given kind. */")
	fmt.Fprintln(src, "public static NodeKind nodeKind(String k) { return NODE_KINDS.getOrDefault(k, NodeKind.UNKNOWN_NODE_KIND); }\n")

	fmt.Fprintln(src, "/** Returns the schema {@link Subkind} for the given kind. */")
	fmt.Fprintln(src, "public static Subkind subkind(String k) { return SUBKINDS.getOrDefault(k, Subkind.UNKNOWN_SUBKIND); }\n")

	fmt.Fprintln(src, "/** Returns the schema {@link EdgeKind} for the given kind. */")
	fmt.Fprintln(src, "public static EdgeKind edgeKind(String k) { return EDGE_KINDS.getOrDefault(k, EdgeKind.UNKNOWN_EDGE_KIND); }\n")

	fmt.Fprintln(src, "/** Returns the schema {@link FactName} for the given name. */")
	fmt.Fprintln(src, "public static FactName factName(String n) { return FACT_NAMES.getOrDefault(n, FactName.UNKNOWN_FACT_NAME); }\n")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link NodeKind}. */")
	fmt.Fprintln(src, "public static String nodeKindString(NodeKind k) { return NODE_KINDS.inverse().getOrDefault(k, \"\"); }\n")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link Subkind}. */")
	fmt.Fprintln(src, "public static String subkindString(Subkind k) { return SUBKINDS.inverse().getOrDefault(k, \"\"); }\n")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link EdgeKind}. */")
	fmt.Fprintln(src, "public static String edgeKindString(EdgeKind k) { return EDGE_KINDS.inverse().getOrDefault(k, \"\"); }\n")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link FactName}. */")
	fmt.Fprintln(src, "public static String factNameString(FactName n) { return FACT_NAMES.inverse().getOrDefault(n, \"\"); }\n")

	fmt.Fprintln(src, "}")

	if err := ioutil.WriteFile(*outputPath, []byte(src.String()), 0644); err != nil {
		log.Fatalf("Writing Java output: %v", err)
	}
}
