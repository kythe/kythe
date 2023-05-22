/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Binary directory_indexer produces a set of Entry protos representing the
// files in the given directories.
//
// For instance, a file 'kythe/javatests/com/google/devtools/kythe/util/BUILD' would produce two entries:
//
//	{
//	  "fact_name": "/kythe/node/kind",
//	  "fact_value": "file",
//	  "source": {
//	    "signature": "c2b0d93b83c1b0e22fd564278be1b0373b1dcb67ff3bb77c2f29df7c393fe580",
//	    "corpus": "kythe",
//	    "root": "",
//	    "path": "kythe/javatests/com/google/devtools/kythe/util/BUILD",
//	    "language": ""
//	  }
//	}
//	{
//	  "fact_name": "/kythe/text",
//	  "fact_value": "...",
//	  "source": {
//	    "signature": "c2b0d93b83c1b0e22fd564278be1b0373b1dcb67ff3bb77c2f29df7c393fe580",
//	    "corpus": "kythe",
//	    "root": "",
//	    "path": "kythe/javatests/com/google/devtools/kythe/util/BUILD",
//	    "language": ""
//	  }
//	}
//
// Usage:
//
//	directory_indexer --corpus kythe --root kythe ~/repo/kythe/ \
//	  --exclude '^buildtools,^bazel-,^third_party,~$,#$,(^|/)\.'
package main

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"flag"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/vnameutil"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	flag.Usage = flagutil.SimpleUsage("Produce a stream of entries representing the files in the given directories",
		"[--verbose] [--emit_irregular] [--vnames path] [--exclude re0,re1,...,reN] [directories]")
}

var (
	vnamesConfigPath = flag.String("vnames", "", "Path to JSON VNames configuration")
	exclude          = flag.String("exclude", "", "Comma-separated list of exclude regexp patterns")
	verbose          = flag.Bool("verbose", false, "Print verbose logging")
	emitIrregular    = flag.Bool("emit_irregular", false, "Emit nodes for irregular files")
)

var (
	kindLabel = "/kythe/node/kind"
	textLabel = "/kythe/text"

	fileKind = []byte("file")
)

var w = delimited.NewWriter(os.Stdout)

func emitEntry(v *spb.VName, label string, value []byte) error {
	return w.PutProto(&spb.Entry{Source: v, FactName: label, FactValue: value})
}

var (
	fileRules vnameutil.Rules
	excludes  []*regexp.Regexp
)

func emitPath(path string, info os.FileInfo, err error) error {
	if err != nil {
		return err
	}
	if info.IsDir() || !(*emitIrregular || info.Mode().IsRegular()) {
		return nil
	}
	for _, re := range excludes {
		if re.MatchString(path) {
			return nil
		}
	}

	if *verbose {
		log.Infof("Reading/emitting %s", path)
	}
	contents, err := vfs.ReadFile(context.Background(), path)
	if err != nil {
		return err
	}
	vName := fileRules.ApplyDefault(path, new(spb.VName))

	digest := sha256.Sum256(contents)
	vName.Signature = hex.EncodeToString(digest[:])

	if vName.Path == "" {
		vName.Path = path
	}

	if err := emitEntry(vName, kindLabel, fileKind); err != nil {
		return err
	}
	return emitEntry(vName, textLabel, contents)
}

func main() {
	flag.Parse()

	if *exclude != "" {
		for _, pattern := range strings.Split(*exclude, ",") {
			excludes = append(excludes, regexp.MustCompile(pattern))
		}
	}

	if data, err := vfs.ReadFile(context.Background(), *vnamesConfigPath); err != nil {
		log.Fatalf("Unable to read VNames config file %q: %v", *vnamesConfigPath, err)
	} else if rules, err := vnameutil.ParseRules(data); err != nil {
		log.Fatalf("Invalid VName rules: %v", err)
	} else {
		fileRules = rules
	}

	dirs := flag.Args()
	if len(dirs) == 0 {
		dirs = []string{"."}
	}

	for _, dir := range dirs {
		if err := filepath.Walk(dir, emitPath); err != nil {
			log.Fatalf("Error walking %s: %v", dir, err)
		}
	}
}
