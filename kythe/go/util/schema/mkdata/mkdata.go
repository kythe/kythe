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

// Program mkdata parses the kythe.proto.schema.Metadata from the Kythe
// schema.proto file descriptor into a Go/Java source file that can be compiled
// into the schema util package.
package main

import (
	"bytes"
	"compress/gzip"
	"flag"
	"fmt"
	"go/format"
	"io/ioutil"
	"reflect"
	"sort"
	"strings"

	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"

	dpb "github.com/golang/protobuf/protoc-gen-go/descriptor"
	scpb "kythe.io/kythe/proto/schema_go_proto"
)

var (
	language    = flag.String("language", "", "Generated target language (supported: go java)")
	outputPath  = flag.String("output", "", "Path of output source file")
	packageName = flag.String("package", "", "Package name to generate")
)

// A SchemaIndex is a glossary of Kythe enum values and their labels.
type SchemaIndex struct {
	NodeKinds map[string]scpb.NodeKind
	Subkinds  map[string]scpb.Subkind
	EdgeKinds map[string]scpb.EdgeKind
	FactNames map[string]scpb.FactName
}

func main() {
	flag.Parse()
	switch {
	case *outputPath == "":
		log.Fatal("You must provide a source -output path")
	case *packageName == "":
		log.Fatal("You must provide a source -package name")
	}

	protoFile := scpb.E_Metadata.Filename
	rd, err := gzip.NewReader(bytes.NewReader(proto.FileDescriptor(protoFile)))
	if err != nil {
		log.Fatalf("Failed to read schema.proto file descriptor: %v", err)
	}
	rec, err := ioutil.ReadAll(rd)
	if err != nil {
		log.Fatalf("Failed to decompress schema.proto file descriptor: %v", err)
	}
	var fd dpb.FileDescriptorProto
	if err := proto.Unmarshal(rec, &fd); err != nil {
		log.Fatalf("Failed to unmarshal schema.proto file descriptor: %v", err)
	}

	index := &SchemaIndex{
		NodeKinds: make(map[string]scpb.NodeKind),
		Subkinds:  make(map[string]scpb.Subkind),
		EdgeKinds: make(map[string]scpb.EdgeKind),
		FactNames: make(map[string]scpb.FactName),
	}
	for _, enum := range fd.EnumType {
		for _, val := range enum.Value {
			ext, err := proto.GetExtension(val.Options, scpb.E_Metadata)
			if err == nil {
				md := ext.(*scpb.Metadata)
				switch enum.GetName() {
				case "EdgeKind":
					index.EdgeKinds[md.Label] = scpb.EdgeKind(val.GetNumber())
				case "NodeKind":
					index.NodeKinds[md.Label] = scpb.NodeKind(val.GetNumber())
				case "FactName":
					index.FactNames[md.Label] = scpb.FactName(val.GetNumber())
				case "Subkind":
					index.Subkinds[md.Label] = scpb.Subkind(val.GetNumber())
				default:
					log.Errorf("unknown Kythe enum %s with Metadata: %+v", enum.GetName(), md)
				}
			}
		}
	}

	switch *language {
	case "go":
		generateGo(protoFile, index)
	case "java":
		generateJava(protoFile, index)
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

func generateGo(protoFile string, index *SchemaIndex) {
	src := new(strings.Builder)
	fmt.Fprintln(src, copyrightHeader)
	fmt.Fprintf(src, "\n\npackage %s\n", *packageName)
	fmt.Fprintf(src, `
// This is a generated file -- do not edit it by hand.
// Input file: %s

`, protoFile)

	fmt.Fprintln(src, `import scpb "kythe.io/kythe/proto/schema_go_proto"`)
	fmt.Fprintln(src, `var (`)

	fmt.Fprintln(src, "nodeKinds = map[string]scpb.NodeKind{")
	nodeKindsRev := make(map[scpb.NodeKind]string)
	for _, name := range stringset.FromKeys(index.NodeKinds).Elements() {
		enum := index.NodeKinds[name]
		fmt.Fprintf(src, "%q: %d,\n", name, enum)
		nodeKindsRev[enum] = name
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nsubkinds = map[string]scpb.Subkind{")
	subkindsRev := make(map[scpb.Subkind]string)
	for _, name := range stringset.FromKeys(index.Subkinds).Elements() {
		enum := index.Subkinds[name]
		fmt.Fprintf(src, "%q: %d,\n", name, enum)
		subkindsRev[enum] = name
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nfactNames = map[string]scpb.FactName{")
	factNamesRev := make(map[scpb.FactName]string)
	for _, name := range stringset.FromKeys(index.FactNames).Elements() {
		enum := index.FactNames[name]
		fmt.Fprintf(src, "%q: %d,\n", name, enum)
		factNamesRev[enum] = name
	}
	fmt.Fprintln(src, "}")

	fmt.Fprintln(src, "\nedgeKinds = map[string]scpb.EdgeKind{")
	edgeKindsRev := make(map[scpb.EdgeKind]string)
	for _, name := range stringset.FromKeys(index.EdgeKinds).Elements() {
		enum := index.EdgeKinds[name]
		fmt.Fprintf(src, "%q: %d,\n", name, enum)
		edgeKindsRev[enum] = name
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
func sortedKeys(v any) []any {
	keys := reflect.ValueOf(v).MapKeys()
	sort.Slice(keys, func(i, j int) bool {
		a := keys[i].Convert(u32).Interface().(uint32)
		b := keys[j].Convert(u32).Interface().(uint32)
		return a < b
	})
	vals := make([]any, len(keys))
	for i, key := range keys {
		vals[i] = key.Interface()
	}
	return vals
}

func generateJava(protoFile string, index *SchemaIndex) {
	src := new(strings.Builder)
	fmt.Fprintln(src, copyrightHeader)
	fmt.Fprintln(src, "\n// This is a generated file -- do not edit it by hand.")
	fmt.Fprintf(src, "// Input file: %s\n\n", protoFile)
	fmt.Fprintf(src, "package %s;\n", *packageName)

	fmt.Fprintln(src, "import com.google.common.collect.ImmutableBiMap;")
	fmt.Fprintln(src, "import com.google.devtools.kythe.proto.Schema.Edge;")
	fmt.Fprintln(src, "import com.google.devtools.kythe.proto.Schema.EdgeKind;")
	fmt.Fprintln(src, "import com.google.devtools.kythe.proto.Schema.Fact;")
	fmt.Fprintln(src, "import com.google.devtools.kythe.proto.Schema.FactName;")
	fmt.Fprintln(src, "import com.google.devtools.kythe.proto.Schema.NodeKind;")
	fmt.Fprintln(src, "import com.google.devtools.kythe.proto.Schema.Subkind;")
	fmt.Fprintln(src, "import com.google.protobuf.ByteString;")

	fmt.Fprintln(src, "/** Utility for handling schema strings and their corresponding protocol buffer enums. */")
	fmt.Fprintln(src, "public final class Schema {")
	fmt.Fprintln(src, "private Schema() {}")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, NodeKind> NODE_KINDS = ImmutableBiMap.<String, NodeKind>builder()")
	for _, name := range stringset.FromKeys(index.NodeKinds).Elements() {
		enum := index.NodeKinds[name]
		fmt.Fprintf(src, ".put(%q, NodeKind.%s)\n", name, enum)
	}
	fmt.Fprintln(src, ".build();")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, Subkind> SUBKINDS = ImmutableBiMap.<String, Subkind>builder()")
	for _, name := range stringset.FromKeys(index.Subkinds).Elements() {
		enum := index.Subkinds[name]
		fmt.Fprintf(src, ".put(%q, Subkind.%s)\n", name, enum)
	}
	fmt.Fprintln(src, ".build();")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, EdgeKind> EDGE_KINDS = ImmutableBiMap.<String, EdgeKind>builder()")
	for _, name := range stringset.FromKeys(index.EdgeKinds).Elements() {
		enum := index.EdgeKinds[name]
		fmt.Fprintf(src, ".put(%q, EdgeKind.%s)\n", name, enum)
	}
	fmt.Fprintln(src, ".build();")

	fmt.Fprintln(src, "private static final ImmutableBiMap<String, FactName> FACT_NAMES = ImmutableBiMap.<String, FactName>builder()")
	for _, name := range stringset.FromKeys(index.FactNames).Elements() {
		enum := index.FactNames[name]
		fmt.Fprintf(src, ".put(%q, FactName.%s)\n", name, enum)
	}
	fmt.Fprintln(src, ".build();")

	fmt.Fprintln(src, "/** Returns the schema {@link NodeKind} for the given kind. */")
	fmt.Fprintln(src, "public static NodeKind nodeKind(String k) { return NODE_KINDS.getOrDefault(k, NodeKind.UNKNOWN_NODE_KIND); }")

	fmt.Fprintln(src, "/** Returns the schema {@link NodeKind} for the given kind. */")
	fmt.Fprintln(src, "public static NodeKind nodeKind(ByteString k) { return nodeKind(k.toStringUtf8()); }")

	fmt.Fprintln(src, "/** Returns the schema {@link Subkind} for the given kind. */")
	fmt.Fprintln(src, "public static Subkind subkind(String k) { return SUBKINDS.getOrDefault(k, Subkind.UNKNOWN_SUBKIND); }")

	fmt.Fprintln(src, "/** Returns the schema {@link Subkind} for the given kind. */")
	fmt.Fprintln(src, "public static Subkind subkind(ByteString k) { return subkind(k.toStringUtf8()); }")

	fmt.Fprintln(src, "/** Returns the schema {@link EdgeKind} for the given kind. */")
	fmt.Fprintln(src, "public static EdgeKind edgeKind(String k) { return EDGE_KINDS.getOrDefault(k, EdgeKind.UNKNOWN_EDGE_KIND); }")

	fmt.Fprintln(src, "/** Returns the schema {@link FactName} for the given name. */")
	fmt.Fprintln(src, "public static FactName factName(String n) { return FACT_NAMES.getOrDefault(n, FactName.UNKNOWN_FACT_NAME); }")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link NodeKind}. */")
	fmt.Fprintln(src, "public static String nodeKindString(NodeKind k) { return NODE_KINDS.inverse().getOrDefault(k, \"\"); }")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link Subkind}. */")
	fmt.Fprintln(src, "public static String subkindString(Subkind k) { return SUBKINDS.inverse().getOrDefault(k, \"\"); }")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link EdgeKind}. */")
	fmt.Fprintln(src, "public static String edgeKindString(EdgeKind k) { return EDGE_KINDS.inverse().getOrDefault(k, \"\"); }")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link Edge}'s kind. */")
	fmt.Fprintln(src, "public static String edgeKindString(Edge e) { return e.getKytheKind().equals(EdgeKind.UNKNOWN_EDGE_KIND) ? e.getGenericKind() : edgeKindString(e.getKytheKind()); }")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link FactName}. */")
	fmt.Fprintln(src, "public static String factNameString(FactName n) { return FACT_NAMES.inverse().getOrDefault(n, \"\"); }")

	fmt.Fprintln(src, "/** Returns the string representation of the given {@link Fact}'s name. */")
	fmt.Fprintln(src, "public static String factNameString(Fact f) { return f.getKytheName().equals(FactName.UNKNOWN_FACT_NAME) ? f.getGenericName() : factNameString(f.getKytheName()); }")

	fmt.Fprintln(src, "}")

	if err := ioutil.WriteFile(*outputPath, []byte(src.String()), 0644); err != nil {
		log.Fatalf("Writing Java output: %v", err)
	}
}
