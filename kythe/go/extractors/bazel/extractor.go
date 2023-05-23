/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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
package bazel // import "kythe.io/kythe/go/extractors/bazel"

import (
	"context"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"time"

	"kythe.io/kythe/go/extractors/bazel/treeset"
	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/vnameutil"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"
	"golang.org/x/sync/errgroup"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
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
//	CheckAction .. CheckInputs/Env .. Fetch .. Fixup
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

	// If set, this function is called with the updated compilation prior to
	// returning it, and may edit the result. If the function reports an error,
	// that error is propagated along with the compilation.
	FixUnit func(*apb.CompilationUnit) error

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

func (c *Config) fixup(cu *apb.CompilationUnit) error {
	if fix := c.FixUnit; fix != nil {
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

func (c *Config) logPrintf(msg string, args ...any) {
	if c.Verbose {
		log.Infof(msg, args...)
	}
}

// ExtractToKzip extracts a spawn action through c and writes the
// results to the specified output file in kzip format. The outputPath
// must have a suffix of ".kzip".
func (c *Config) ExtractToKzip(ctx context.Context, ai *ActionInfo, outputPath string) error {
	if ext := filepath.Ext(outputPath); ext != ".kzip" {
		return fmt.Errorf("unknown output extension %q", ext)
	}
	w, err := NewKZIP(outputPath)
	if err != nil {
		return fmt.Errorf("creating kzip writer: %v", err)
	}
	if _, err := c.ExtractToFile(ctx, ai, w); err != nil {
		return fmt.Errorf("extracting: %v", err)
	}
	if err := w.Close(); err != nil {
		return fmt.Errorf("closing output: %v", err)
	}
	return nil
}

// ExtractToFile extracts a compilation from the specified extra action info,
// and writes it along with its required inputs to w. The unit digest of the
// stored compilation is returned.
func (c *Config) ExtractToFile(ctx context.Context, info *ActionInfo, w *kzip.Writer) (string, error) {
	cu, err := c.extract(ctx, info, func(ri *apb.CompilationUnit_FileInput, r io.Reader) error {
		digest, err := w.AddFile(r)
		if err == nil {
			ri.Info.Digest = digest
		}
		return err
	})
	if err != nil {
		return "", err
	}
	return w.AddUnit(cu, nil)
}

type fileReader func(*apb.CompilationUnit_FileInput, io.Reader) error

// extract extracts a compilation from the specified extra action info.
func (c *Config) extract(ctx context.Context, info *ActionInfo, file fileReader) (*apb.CompilationUnit, error) {
	log.Infof("Extracting XA for %q with %d inputs", info.Target, len(info.Inputs))
	if err := c.checkAction(ctx, info); err != nil {
		return nil, err
	}

	if c.Corpus == "" {
		c.Corpus = c.inferCorpus(info)
	}

	// Construct the basic compilation.
	cu := &apb.CompilationUnit{
		VName: &spb.VName{
			Language: c.Language,
			Corpus:   c.Corpus,
		},
		Argument: info.Arguments,
	}

	// Capture the primary output path. Although the action has room for
	// multiple outputs, we expect only one to be set in practice. It's
	// harmless if there are more, though, so don't fail for that.
	if len(info.Outputs) > 0 {
		cu.OutputKey = info.Outputs[0]
	}

	// Capture environment variables.
	for name, value := range info.Environment {
		if c.checkEnv(name, value) {
			cu.Environment = append(cu.Environment, &apb.CompilationUnit_Env{
				Name:  name,
				Value: value,
			})
		}
	}

	// Capture the build system details.
	if err := SetTarget(info.Target, info.Rule, cu); err != nil {
		log.Errorf("Adding build details: %v", err)
	}

	// Load and populate file contents and required inputs. First scan the
	// inputs and filter out which ones we actually want to keep by path
	// inspection; then load the contents concurrently.
	sort.Strings(info.Inputs) // ensure a consistent order
	inputs := c.classifyInputs(ctx, info, cu)

	start := time.Now()
	if err := c.fetchInputs(ctx, inputs, func(i int, r io.Reader) error {
		return file(cu.RequiredInput[i], r)
	}); err != nil {
		return nil, fmt.Errorf("reading input files failed: %v", err)
	}
	log.Infof("Finished reading required inputs [%v elapsed]", time.Since(start))
	if err := c.fixup(cu); err != nil {
		return nil, err
	}
	log.Infof("Found %d required inputs, %d source files", len(cu.RequiredInput), len(cu.SourceFile))
	return cu, nil
}

// fetchInputs concurrently fetches the contents of all the specified file
// paths. An open reader for each file is passed sequentially to the file
// callback along with its path's offset in the input slice. If the callback
// returns an error, that error is propagated.
func (c *Config) fetchInputs(ctx context.Context, paths []string, file func(int, io.Reader) error) error {
	// Fetch concurrently. Each element of the proto slices is accessed by a
	// single goroutine corresponding to its index.

	g, gCtx := errgroup.WithContext(ctx)

	files := make([]chan io.ReadCloser, len(paths))
	for i := range paths {
		files[i] = make(chan io.ReadCloser)
	}
	g.Go(func() error {
		// Pass each file Reader to the callback sequentially
		for i, ch := range files {
			select {
			case <-gCtx.Done():
				return gCtx.Err()
			case rc := <-ch:
				err := file(i, rc)
				rc.Close()
				if err != nil {
					return err
				}
			}
		}
		return nil
	})

	throttle := make(chan struct{}, 256)
	for i, path := range paths {
		i, path := i, path
		throttle <- struct{}{}
		g.Go(func() error {
			defer func() { <-throttle }()
			rc, err := c.openRead(gCtx, path)
			if err != nil {
				log.Errorf("Reading input file: %v", err)
				return err
			}
			select {
			case <-gCtx.Done():
				rc.Close()
				return gCtx.Err()
			case files[i] <- rc:
				return nil
			}
		})
	}
	return g.Wait()
}

func (c *Config) inferCorpus(info *ActionInfo) string {
	var sourceCorpora stringset.Set
	for _, in := range info.Inputs {
		path, ok := c.checkInput(in)
		if !ok || !c.isSource(path) {
			continue
		}

		vname, ok := c.Rules.Apply(path)
		if ok && vname.Corpus != "" {
			sourceCorpora.Add(vname.Corpus)
		}
	}

	corpora := sourceCorpora.Elements()
	if len(sourceCorpora) != 1 {
		log.Warningf("could not infer compilation corpus from source files: %v", corpora)
		return ""
	}

	corpus := corpora[0]
	c.logPrintf("Inferred compilation corpus from source files: %q", corpus)
	return corpus
}

// classifyInputs updates unit to add required inputs for each matching path
// and to identify source inputs according to the rules of c. The filtered
// complete list of inputs paths is returned.
func (c *Config) classifyInputs(ctx context.Context, info *ActionInfo, unit *apb.CompilationUnit) []string {
	var inputs, sourceFiles stringset.Set
	// Inputs might be file or directories https://docs.bazel.build/versions/master/glossary.html#artifact
	// So we need to expand directories and process only files.
	for _, in := range treeset.ExpandDirectories(ctx, info.Inputs) {
		path, ok := c.checkInput(in)
		if ok {
			if !inputs.Add(path) {
				continue // don't re-add files we've already seen
			}
			if c.isSource(path) {
				sourceFiles.Add(path)
				c.logPrintf("Matched source file from inputs: %q", path)
			}
			vname, ok := c.Rules.Apply(path)
			if !ok {
				vname = &spb.VName{Corpus: c.Corpus, Path: path}
			} else if vname.Corpus == "" {
				vname.Corpus = c.Corpus
			}

			// Add the skeleton of a required input carrying the vname.
			// File info (path, digest) are populated during fetch.
			unit.RequiredInput = append(unit.RequiredInput, &apb.CompilationUnit_FileInput{
				VName: vname,
				Info:  &apb.FileInfo{Path: path},
			})
		} else {
			c.logPrintf("Excluding input file: %q", in)
		}
	}
	for _, src := range treeset.ExpandDirectories(ctx, info.Sources) {
		if inputs.Contains(src) {
			c.logPrintf("Matched source file from action: %q", src)
			sourceFiles.Add(src)
		}
	}
	unit.SourceFile = sourceFiles.Elements()
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
