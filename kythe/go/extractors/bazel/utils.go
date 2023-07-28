/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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
	"os"
	"regexp"
	"strings"
	"time"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/datasize"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/ptypes"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	bipb "kythe.io/kythe/proto/buildinfo_go_proto"
	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
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
	log.Infof("Finished writing output [%v elapsed]", time.Since(start))
	return nil
}

// NewKZIP creates a kzip writer at path, replacing any existing file at that
// location. Closing the returned writer also closes the underlying file.
func NewKZIP(path string) (*kzip.Writer, error) {
	f, err := os.Create(path)
	if err != nil {
		return nil, fmt.Errorf("creating output file: %v", err)
	}
	w, err := kzip.NewWriteCloser(f)
	if err != nil {
		f.Close()
		return nil, err
	}
	return w, nil
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
	log.Infof("Read %d bytes from extra action file %q", len(xa), path)
	return &info, nil
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

// SetTarget adds a details message to unit with the specified build target
// name and rule type.
func SetTarget(target, rule string, unit *apb.CompilationUnit) error {
	if target != "" || rule != "" {
		return AddDetail(unit, &bipb.BuildDetails{
			BuildTarget: target,
			RuleType:    rule,
		})
	}
	return nil
}

// AddDetail adds the specified message to the details field of unit.
func AddDetail(unit *apb.CompilationUnit, msg proto.Message) error {
	det, err := ptypes.MarshalAny(msg)
	if err == nil {
		unit.Details = append(unit.Details, det)
	}
	return err
}

// FindSourceArgs returns a fixup that scans the argument list of a compilation
// unit for strings matching r. Any that are found, and which also match the
// names of required input files, are added to the source files of the unit.
func FindSourceArgs(r *regexp.Regexp) func(*apb.CompilationUnit) error {
	return func(cu *apb.CompilationUnit) error {
		var inputs stringset.Set
		for _, ri := range cu.RequiredInput {
			inputs.Add(ri.Info.GetPath())
		}

		srcs := stringset.New(cu.SourceFile...)
		for _, arg := range cu.Argument {
			if r.MatchString(arg) && inputs.Contains(arg) {
				srcs.Add(arg)
			}
		}
		cu.SourceFile = srcs.Elements()
		return nil
	}
}

// CheckFileSize returns true if maxSize == 0 or
// the size of the file at path exists and is less than or equal to maxSize.
func CheckFileSize(path string, maxSize datasize.Size) bool {
	if maxSize == 0 {
		return true
	}
	info, err := os.Stat(path)
	if err != nil {
		return false
	}
	sz := info.Size()
	if sz <= 0 {
		return true
	}
	return datasize.Size(sz) <= maxSize
}
