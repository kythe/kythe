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

package bazel

// Common support code for binaries built around this library.

import (
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"strings"
	"time"

	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/util/vnameutil"

	xapb "kythe.io/third_party/bazel/extra_actions_base_proto"
)

// Write writes w to path, creating the path if necessary and replacing any
// existing file at that location.
func Write(w io.WriterTo, path string) error {
	start := time.Now()
	f, err := os.Create(path)
	if err != nil {
		return fmt.Errorf("creating output file: %v", err)
	}
	if _, err := w.WriteTo(f); err != nil {
		return fmt.Errorf("writing output file: %v", err)
	} else if err := f.Close(); err != nil {
		return fmt.Errorf("closing output file: %v", err)
	}
	log.Printf("Finished writing output [%v elapsed]", time.Since(start))
	return nil
}

// LoadAction loads and parses a wire-format ExtraActionInfo message from the
// specified path, or aborts the program with an error.
func LoadAction(path string) (*xapb.ExtraActionInfo, error) {
	xa, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("reading extra action info: %v", err)
	}
	var info xapb.ExtraActionInfo
	if err := proto.Unmarshal(xa, &info); err != nil {
		return nil, fmt.Errorf("parsing extra action info: %v", err)
	}
	log.Printf("Read %d bytes from extra action file %q", len(xa), path)
	return &info, nil
}

// LoadRules loads and parses the vname mapping rules in path.
// If path == "", this returns nil without error (no rules).
func LoadRules(path string) (vnameutil.Rules, error) {
	if path == "" {
		return nil, nil
	}
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("reading vname rules: %v", err)
	}
	rules, err := vnameutil.ParseRules(data)
	if err != nil {
		return nil, fmt.Errorf("parsing vname rules: %v", err)
	}
	return rules, nil
}

// PackageName extracts the base name of a Bazel package from a target label,
// for example //foo/bar:baz ⇒ foo/bar.
func PackageName(label string) string {
	return strings.SplitN(strings.TrimPrefix(label, "//"), ":", 2)[0]
}

// PathInPackage reports whether path contains pkg as a directory fragment.
func PathInPackage(path, pkg string) bool {
	return strings.Contains(path, "/"+pkg+"/") || strings.HasPrefix(path, pkg+"/")
}
