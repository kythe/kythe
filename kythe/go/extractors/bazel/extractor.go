/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// Package bazel implements the internal plumbing of a configurable Bazel
// compilation unit extractor.
package bazel

import (
	"context"
	"fmt"
	"io"
	"log"
	"os"
	"sort"
	"time"

	"bitbucket.org/creachadair/stringset"

	"github.com/golang/protobuf/proto"
	"golang.org/x/sync/errgroup"

	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/util/vnameutil"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
	xapb "kythe.io/third_party/bazel/extra_actions_base_proto"
)

// A Config carries settings that control the extraction process.
//
// By default, all input files are captured as required inputs, all specified
// environment variables are stored, all command-line arguments are recorded,
// and the "owner" of the extra action is marked as the build target in a build
// details message.
//
// The caller may override these behaviours by providing callbacks to handle
// various stages of the extraction process. Schematically, the extractor does
// the following steps:
//
//    CheckAction .. CheckInputs/Env .. Fetch .. Fixup
//
// The "CheckAction" stage gives the caller an opportunity to preprocess the
// action and decide whether to continue. The caller may modify the ActionInfo
// during this process if it wishes.
//
// Next, each input file is checked for inclusion in the compilation, and for
// whether it should be counted as a source file for the compilation. Also,
// each environment variable is checked for inclusion.
//
// The "Fetch" stage reads the contents of the required input files selected
// during the previous stage, computes their digests, and packs them into the
// compilation record.
//
// Finally, the "Fixup" stage gives the caller a final opportunity to edit the
// resulting compilation record before it is returned.
type Config struct {
	Corpus   string          // the default corpus label to use
	Language string          // the language label to apply
	Rules    vnameutil.Rules // rules for rewriting file VNames
	Verbose  bool            // whether to emit verbose (per-file) logging

	// If set, this function checks whether the given spawn action should be
	// further processed. If it returns an error, the action will be rejected.
	// Otherwise, all actions will be processed.
	//
	// This function may modify its argument, and such changes will be
	// preserved as this is invoked before further processing.
	CheckAction func(context.Context, *ActionInfo) error

	// If set, this function reports whether an input path should be kept in
	// the resulting compilation unit, and returns an optionally-modified
	// version of the path. Otherwise, all inputs are kept.
	CheckInput func(string) (string, bool)

	// If set, this function reports whether an environment variable should be
	// kept in the resulting compilation unit. Otherwise, all environment
	// variables are kept.
	CheckEnv func(name, value string) bool

	// If set, this function reports whether an input path should be considered
	// a source file. Otherwise, no inputs are recorded as sources. The path
	// given to this function reflects any modifications made by CheckInput.
	IsSource func(string) bool

	// If set, this function is called with the completed compilation prior to
	// returning it, and may edit the result. If the function reports an error,
	// that error is propagated along with the compilation.
	Fixup func(*kindex.Compilation) error

	// If set, this function is used to open files for reading.  If nil,
	// os.Open is used.
	OpenRead func(context.Context, string) (io.ReadCloser, error)
}

func (c *Config) checkAction(ctx context.Context, info *ActionInfo) error {
	if check := c.CheckAction; check != nil {
		return check(ctx, info)
	}
	return nil
}

func (c *Config) checkInput(path string) (string, bool) {
	if keep := c.CheckInput; keep != nil {
		return keep(path)
	}
	return path, true
}

func (c *Config) checkEnv(name, value string) bool {
	if keep := c.CheckEnv; keep != nil {
		return keep(name, value)
	}
	return false
}

func (c *Config) isSource(path string) bool {
	if src := c.IsSource; src != nil {
		return src(path)
	}
	return false
}

func (c *Config) fixup(cu *kindex.Compilation) error {
	if fix := c.Fixup; fix != nil {
		return fix(cu)
	}
	return nil
}

func (c *Config) openRead(ctx context.Context, path string) (io.ReadCloser, error) {
	if open := c.OpenRead; open != nil {
		return open(ctx, path)
	}
	return os.Open(path)
}

func (c *Config) logPrintf(msg string, args ...interface{}) {
	if c.Verbose {
		log.Printf(msg, args...)
	}
}

// Extract extracts a compilation from the specified extra action info.
func (c *Config) Extract(ctx context.Context, info *ActionInfo) (*kindex.Compilation, error) {
	log.Printf("Extracting XA for %q with %d inputs", info.Target, len(info.Inputs))
	if err := c.checkAction(ctx, info); err != nil {
		return nil, err
	}

	// Construct the basic compilation.
	cu := &kindex.Compilation{
		Proto: &apb.CompilationUnit{
			VName: &spb.VName{
				Language: c.Language,
				Corpus:   c.Corpus,
			},
			Argument: info.Arguments,
		},
	}

	// Capture the primary output path.  Although the action has room for
	// multiple outputs, we expect only one to be set in practice.  It's
	// harmless if there are more, though, so don't fail for that.
	if len(info.Outputs) > 0 {
		cu.Proto.OutputKey = info.Outputs[0]
	}

	// Capture environment variables.
	for name, value := range info.Environment {
		if c.checkEnv(name, value) {
			cu.Proto.Environment = append(cu.Proto.Environment, &apb.CompilationUnit_Env{
				Name:  name,
				Value: value,
			})
		}
	}

	// Capture the build system details.
	if err := SetTarget(info.Target, info.Rule, cu); err != nil {
		log.Printf("ERROR: Adding build details: %v", err)
	}

	// Load and populate file contents and required inputs.  First scan the
	// inputs and filter out which ones we actually want to keep by path
	// inspection; then load the contents concurrently.
	sort.Strings(info.Inputs) // ensure a consistent order
	inputs := c.ClassifyInputs(info, cu)

	start := time.Now()
	fileData, err := c.FetchInputs(ctx, inputs)
	if err != nil {
		log.Fatalf("Reading input files failed: %v", err)
	}
	log.Printf("Finished reading required inputs [%v elapsed]", time.Since(start))

	// Update the required inputs with file info.
	for i, fd := range fileData {
		cu.Proto.RequiredInput[i].Info = fd.Info
	}
	cu.Files = fileData
	return cu, c.fixup(cu)
}

// readFileData fetches the contents of the file at path and returns a FileData
// message populated with its content and digest.
func (c *Config) readFileData(ctx context.Context, path string) (*apb.FileData, error) {
	f, err := c.openRead(ctx, path)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	return kindex.FileData(path, f)
}

// FetchInputs concurrently fetches the contents of all the specified file
// paths.  All files are attempted regardless of error, but nil is only
// returned if all fetches were successful.
func (c *Config) FetchInputs(ctx context.Context, paths []string) ([]*apb.FileData, error) {
	// Fetch concurrently. Each element of the proto slices is accessed by a
	// single goroutine corresponding to its index.

	fileData := make([]*apb.FileData, len(paths))
	var g errgroup.Group
	for i, path := range paths {
		i, path := i, path
		g.Go(func() error {
			fd, err := c.readFileData(ctx, path)
			if err != nil {
				log.Printf("ERROR: Reading input file: %v", err)
			} else {
				fileData[i] = fd
			}
			return err
		})
	}
	return fileData, g.Wait()
}

// ClassifyInputs updates unit to add required inputs for each matching path
// and to identify source inputs according to the rules of c. The filtered
// complete list of inputs paths is returned.
func (c *Config) ClassifyInputs(info *ActionInfo, unit *kindex.Compilation) []string {
	var inputs, sourceFiles stringset.Set
	for _, in := range info.Inputs {
		path, ok := c.checkInput(in)
		if ok {
			inputs.Add(path)
			if c.isSource(path) {
				sourceFiles.Add(path)
				c.logPrintf("Matched source file from inputs: %q", path)
			}
			vname, ok := c.Rules.Apply(path)
			if !ok {
				vname = &spb.VName{Corpus: c.Corpus, Path: path}
			}

			// Add the skeleton of a required input carrying the vname.
			// File info (path, digest) are populated during fetch.
			unit.Proto.RequiredInput = append(unit.Proto.RequiredInput, &apb.CompilationUnit_FileInput{
				VName: vname,
			})
		} else {
			c.logPrintf("Excluding input file: %q", in)
		}
	}
	for _, src := range info.Sources {
		if inputs.Contains(src) {
			c.logPrintf("Matched source file from action: %q", src)
			sourceFiles.Add(src)
		}
	}
	unit.Proto.SourceFile = sourceFiles.Elements()
	log.Printf("Found %d required inputs, %d source files", len(inputs), len(sourceFiles))
	return inputs.Elements()
}

// ActionInfo represents the action metadata relevant to the extraction process.
type ActionInfo struct {
	Arguments   []string          // command-line arguments
	Inputs      []string          // input file paths
	Outputs     []string          // output file paths
	Sources     []string          // source file paths
	Environment map[string]string // environment variables
	Target      string            // build target name
	Rule        string            // rule class name

	// Paths in Sources are expected to be a subset of inputs. In particular
	// the extractor will keep such a path only if it also appears in the
	// Inputs, and has been selected by the other rules provided by the caller.
	//
	// Such paths, if there are any, are taken in addition to any source files
	// identified by the extraction rules provided by the caller.
}

// Setenv updates the Environment field with the specified key-value pair.
func (a *ActionInfo) Setenv(key, value string) {
	if a.Environment == nil {
		a.Environment = map[string]string{key: value}
	} else {
		a.Environment[key] = value
	}
}

// SpawnAction generates an *ActionInfo from a spawn action.
// It is an error if info does not contain a SpawnInfo.
func SpawnAction(info *xapb.ExtraActionInfo) (*ActionInfo, error) {
	msg, err := proto.GetExtension(info, xapb.E_SpawnInfo_SpawnInfo)
	if err != nil {
		return nil, fmt.Errorf("extra action does not have SpawnInfo: %v", err)
	}
	si := msg.(*xapb.SpawnInfo)
	ai := &ActionInfo{
		Target:    info.GetOwner(),
		Arguments: si.Argument,
		Inputs:    si.InputFile,
		Outputs:   si.OutputFile,
	}
	for _, env := range si.Variable {
		ai.Setenv(env.GetName(), env.GetValue())
	}
	return ai, nil
}
