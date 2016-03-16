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

// Package bazel implements the internal plumbing of a Bazel extractor for Go.
package bazel

import (
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"bitbucket.org/creachadair/shell"
	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/storage/vnameutil"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
	eapb "kythe.io/third_party/bazel/extra_actions_base_proto"
)

// TODO(fromberger): The extractor logic depends on details of the Bazel rule
// implementation, which needs some cleanup.

func osOpen(_ context.Context, path string) (io.ReadCloser, error) { return os.Open(path) }

// A Config carries settings that control the extraction process.
type Config struct {
	Corpus   string          // the default corpus label to use
	Mnemonic string          // the build mnemonic to match (if "", matches all)
	Root     string          // working directory (if "", uses os.Getcwd)
	Rules    vnameutil.Rules // rules for rewriting file VNames

	// If set, this function is used to open files for reading.  If nil,
	// os.Open is used.
	OpenRead func(context.Context, string) (io.ReadCloser, error)
}

// Extract extracts a compilation from the specified extra action info.
func (c *Config) Extract(ctx context.Context, info *eapb.ExtraActionInfo) (*kindex.Compilation, error) {
	si, err := proto.GetExtension(info, eapb.E_SpawnInfo_SpawnInfo)
	if err != nil {
		return nil, fmt.Errorf("extra action does not have SpawnInfo: %v", err)
	}
	spawnInfo := si.(*eapb.SpawnInfo)

	// Verify that the mnemonic is what we expect.
	if m := info.GetMnemonic(); m != c.Mnemonic && c.Mnemonic != "" {
		return nil, fmt.Errorf("mnemonic does not match %q ≠ %q", m, c.Mnemonic)
	}

	// Construct the basic compilation.
	toolArgs := extractToolArgs(spawnInfo.Argument)
	log.Printf("Extracting compilation for %q", info.GetOwner())
	cu := &kindex.Compilation{
		Proto: &apb.CompilationUnit{
			VName: &spb.VName{
				Language:  govname.Language,
				Corpus:    c.Corpus,
				Signature: info.GetOwner(),
			},
			Argument:         toolArgs.fullArgs,
			SourceFile:       toolArgs.sources,
			WorkingDirectory: c.Root,
			Environment: []*apb.CompilationUnit_Env{{
				Name:  "GOROOT",
				Value: toolArgs.goRoot,
			}},
		},
	}

	// Load and populate file contents and required inputs.  Do this in two
	// passes: First scan the inputs and filter out which ones we actually want
	// to keep; then load their contents concurrently.
	var wantPaths []string
	for _, in := range spawnInfo.InputFile {
		if toolArgs.wantInput(in) {
			wantPaths = append(wantPaths, in)
			cu.Files = append(cu.Files, nil)
			cu.Proto.RequiredInput = append(cu.Proto.RequiredInput, nil)
		}
	}

	// Fetch concurrently. Each element of the proto slices is accessed by a
	// single goroutine corresponding to its index.
	log.Printf("Reading file contents for %d required inputs", len(wantPaths))
	start := time.Now()
	var wg sync.WaitGroup
	for i, path := range wantPaths {
		i, path := i, path
		wg.Add(1)
		go func() {
			defer wg.Done()
			fd, err := c.readFile(ctx, path)
			if err != nil {
				log.Fatalf("Unable to read input %q: %v", path, err)
			}
			cu.Files[i] = fd
			cu.Proto.RequiredInput[i] = c.fileDataToInfo(fd)
		}()
	}
	wg.Wait()
	log.Printf("Finished reading required inputs [%v elapsed]", time.Since(start))

	// Set the output path.  Although the SpawnInfo has room for multiple
	// outputs, we expect only one to be set in practice.  It's harmless if
	// there are more, though, so don't fail for that.
	for _, out := range spawnInfo.OutputFile {
		cu.Proto.OutputKey = out
		break
	}

	// Capture environment variables.
	for _, evar := range spawnInfo.Variable {
		if evar.GetName() == "PATH" {
			// TODO(fromberger): Perhaps whitelist or blacklist which
			// environment variables to capture here.
			continue
		}
		cu.Proto.Environment = append(cu.Proto.Environment, &apb.CompilationUnit_Env{
			Name:  evar.GetName(),
			Value: evar.GetValue(),
		})
	}

	return cu, nil
}

// readFile fetches the contents of the file at path and returns a FileData
// message populated with its content, path, and digest.
func (c *Config) readFile(ctx context.Context, path string) (*apb.FileData, error) {
	open := c.OpenRead
	if open == nil {
		open = osOpen
	}
	f, err := open(ctx, path)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	return kindex.FileData(path, f)
}

// fileDataToInfo produces a file info message corresponding to fd, using rules
// to generate the vname and root as the working directory.
func (c *Config) fileDataToInfo(fd *apb.FileData) *apb.CompilationUnit_FileInput {
	path := fd.Info.Path
	if !filepath.IsAbs(path) {
		if abs, err := filepath.Abs(path); err == nil {
			path = abs
		}
	}
	if rel, err := filepath.Rel(c.Root, path); err == nil && c.Root != "" {
		path = rel
	}
	vname, ok := c.Rules.Apply(path)
	if !ok {
		vname = &spb.VName{Corpus: c.Corpus, Path: path}
	}
	return &apb.CompilationUnit_FileInput{
		VName: vname,
		Info:  fd.Info,
	}
}

// toolArgs captures the settings expressed by the Go compiler tool and its
// arguments.
type toolArgs struct {
	fullArgs    []string // complete argument list
	goRoot      string   // the GOROOT path
	importPath  string   // the import path being compiled
	includePath string   // an include path, if set
	outputPath  string   // the output from the compiler
	toolRoot    string   // root directory for compiler/libraries
	useCgo      bool     // whether cgo is enabled
	sources     []string // source file paths
}

// wantInput reports whether path should be included as a required input.
//
// TODO(T110): The current Bazel rules include a lot of excess data, which
// makes the extraction process slow (because we are writing a large number of
// "required inputs" that aren't actually required). Improve communication
// between the rules and the tools so that we can prune out the unnecessary
// packages.
func (g *toolArgs) wantInput(path string) bool {
	// Anything that isn't in the tool root we keep.
	if !strings.HasPrefix(path, g.toolRoot) {
		return true
	}
	switch filepath.Ext(path) {
	case ".a", ".h":
		return true
	default:
		return false
	}
}

// extractToolArgs extracts the build tool arguments from args.
func extractToolArgs(args []string) toolArgs {
	if len(args) == 3 && args[0] == "/bin/bash" && args[1] == "-c" {
		actual, ok := shell.Split(args[2])
		if !ok {
			log.Fatalf("Invalid shell-quoted argument %#q", args[2])
		}
		args = actual
	}

	// Find the Go tool invocation, and return only that portion.
	var result toolArgs
	var wantArg *string
	inTool := false
	for _, arg := range args {
		// Discard arguments until the tool binary is found.
		if !inTool {
			if root := strings.TrimPrefix(arg, "GOROOT="); root != arg {
				result.goRoot = os.ExpandEnv(root)
				continue
			} else if ok, dir := isGoTool(arg); ok {
				inTool = true
				result.toolRoot = dir
			} else {
				continue
			}
		}
		result.fullArgs = append(result.fullArgs, arg)

		// Scan for important flags.
		if wantArg != nil { // capture argument for a previous flag
			*wantArg = arg
			wantArg = nil
			continue
		}
		if arg == "-p" {
			wantArg = &result.importPath
		} else if arg == "-o" {
			wantArg = &result.outputPath
		} else if arg == "-I" {
			wantArg = &result.includePath
		} else if !strings.HasPrefix(arg, "-") && strings.HasSuffix(arg, ".go") {
			result.sources = append(result.sources, arg)
		}
	}
	return result
}

// isGoTool reports whether arg is the path of the "go" build tool, and if so
// returns the enclosing directory.
//
// TODO(fromberger): This is a heuristic that relies on details of the Kythe
// Bazel rules. Make this less brittle.
func isGoTool(arg string) (bool, string) {
	if dir := strings.TrimSuffix(arg, "/bin/go"); dir != arg {
		return true, dir
	}
	return false, ""
}
