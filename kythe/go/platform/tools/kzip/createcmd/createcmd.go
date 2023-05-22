/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Package createcmd provides the kzip command for creating simple kzip archives.
package createcmd // import "kythe.io/kythe/go/platform/tools/kzip/createcmd"

import (
	"context"
	"flag"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/tools/kzip/flags"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/vnameutil"

	"github.com/google/subcommands"

	anypb "github.com/golang/protobuf/ptypes/any"
	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

type createCommand struct {
	cmdutil.Info

	output string
	rules  vnameRules

	uri          kytheURI
	source       flagutil.StringSet
	inputs       flagutil.StringSet
	hasError     bool
	argument     repeatedString
	outputKey    string
	workingDir   string
	entryContext string
	environment  repeatedEnv
	details      repeatedAny
	encoding     flags.EncodingFlag
}

// New creates a new subcommand for merging kzip files.
func New() subcommands.Command {
	return &createCommand{
		Info: cmdutil.NewInfo("create", "create simple kzip archives", `[options] -- arguments*

Construct a kzip file written to -output with the vname specified by -uri.
Each of -source_file, -required_input and -details may be specified multiple times with each
occurrence of the flag being appended to the corresponding field in the compilation unit.
Directories specified in -source_file or -required_input will be added recursively.

Any additional positional arguments are included as arguments in the compilation unit.
`),
		encoding: flags.EncodingFlag{Encoding: kzip.EncodingJSON},
	}
}

// SetFlags implements the subcommands interface and provides command-specific flags
// for creating kzip files.
func (c *createCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.output, "output", "", "Path for output kzip file (required)")
	fs.Var(&c.rules, "rules", "Path to vnames.json file (optional)")

	fs.Var(&c.uri, "uri", "A Kythe URI naming the compilation unit VName (required)")
	fs.Var(&c.source, "source_file", "Repeated paths for input source files or directories (required)")
	fs.Var(&c.inputs, "required_input", "Repeated paths for additional required inputs (optional)")
	fs.BoolVar(&c.hasError, "has_compile_errors", false, "Whether this unit had compilation errors (optional)")
	fs.Var(&c.argument, "argument", "Repeated arguments to add to compilation unit (optional)")
	fs.StringVar(&c.outputKey, "output_key", "", "Name by which the output of this compilation is known to dependents (optional)")
	fs.StringVar(&c.workingDir, "working_directory", "", "Absolute path of the directory from which the build tool was invoked (optional)")
	fs.StringVar(&c.entryContext, "entry_context", "", "Language-specific context to provide the indexer (optional)")
	fs.Var(&c.environment, "env", "Repeated KEY=VALUE pairs of environment variables to add to the compilation unit (optional)")
	fs.Var(&c.details, "details", "Repeated JSON-encoded Any messages to embed as compilation details (optional)")
	fs.Var(&c.encoding, "encoding", "Encoding to use on output, one of JSON, PROTO, or ALL")
}

// Execute implements the subcommands interface and creates the requested file.
func (c *createCommand) Execute(ctx context.Context, fs *flag.FlagSet, _ ...any) subcommands.ExitStatus {
	switch {
	case c.uri.Corpus == "":
		return c.Fail("Missing required -uri")
	case c.output == "":
		return c.Fail("Missing required -output")
	case c.source.Len() == 0:
		return c.Fail("Missing required -source_file")
	}

	opt := kzip.WithEncoding(c.encoding.Encoding)
	out, err := openWriter(ctx, c.output, opt)
	if err != nil {
		return c.Fail("Error opening -output: %v", err)
	}

	// Create a new compilation populating its VName with the values specified
	// within the specified Kythe URI.
	cb := compilationBuilder{&apb.CompilationUnit{
		VName: &spb.VName{
			Corpus:    c.uri.Corpus,
			Language:  c.uri.Language,
			Signature: c.uri.Signature,
			Root:      c.uri.Root,
			Path:      c.uri.Path,
		},
		HasCompileErrors: c.hasError,
		Argument:         append(c.argument, fs.Args()...),
		OutputKey:        c.outputKey,
		WorkingDirectory: c.workingDir,
		EntryContext:     c.entryContext,
		Environment:      c.environment.ToProto(),
		Details:          ([]*anypb.Any)(c.details),
	}, out, &c.rules.Rules}

	sources, err := cb.addFiles(ctx, c.source.Elements())
	if err != nil {
		return c.Fail("Error adding source files: %v", err)
	}
	cb.unit.SourceFile = sources

	if _, err = cb.addFiles(ctx, c.inputs.Elements()); err != nil {
		return c.Fail("Error adding input files: %v", err)
	}

	if err := cb.done(); err != nil {
		return c.Fail("Error writing compilation to -output: %v", err)
	}
	return subcommands.ExitSuccess
}

func openWriter(ctx context.Context, path string, opts ...kzip.WriterOption) (*kzip.Writer, error) {
	out, err := vfs.Create(ctx, path)
	if err != nil {
		return nil, err
	}
	return kzip.NewWriteCloser(out, opts...)
}

type compilationBuilder struct {
	unit  *apb.CompilationUnit
	out   *kzip.Writer
	rules *vnameutil.Rules
}

// addFiles adds the given files as required input.
// If the path is a directory, its contents are added recursively.
// Returns the paths of the non-directory files added.
func (cb *compilationBuilder) addFiles(ctx context.Context, paths []string) ([]string, error) {
	var files []string
	for _, path := range paths {
		f, err := cb.addFile(ctx, path)
		if err != nil {
			return files, err
		}
		files = append(files, f...)
	}
	return files, nil
}

// addFile adds the given file as a required input.
// If the path is a directory, its contents are added recursively.
// Returns the paths of the non-directory files added.
func (cb *compilationBuilder) addFile(ctx context.Context, root string) ([]string, error) {
	var files []string
	err := vfs.Walk(ctx, root, func(path string, info os.FileInfo, err error) error {
		if err != nil || info.IsDir() {
			return err
		}
		input, err := vfs.Open(ctx, path)
		if err != nil {
			return err
		}
		defer input.Close()

		digest, err := cb.out.AddFile(input)
		if err != nil {
			return err
		}

		path = cb.tryMakeRelative(path)
		vname, ok := cb.rules.Apply(path)
		if !ok {
			vname = &spb.VName{
				Corpus: cb.unit.VName.Corpus,
				Root:   cb.unit.VName.Root,
				Path:   path,
			}
		} else if vname.Corpus == "" {
			vname.Corpus = cb.unit.VName.Corpus

		}
		cb.unit.RequiredInput = append(cb.unit.RequiredInput, &apb.CompilationUnit_FileInput{
			VName: vname,
			Info: &apb.FileInfo{
				Path:   path,
				Digest: digest,
			},
		})
		files = append(files, path)
		return nil
	})
	return files, err
}

func (cb *compilationBuilder) done() error {
	_, err := cb.out.AddUnit(cb.unit, nil)
	if err != nil {
		return err
	}
	cb.unit = nil
	return cb.out.Close()
}

// tryeMakeRelative attempts to relativize path against unit.WorkingDirectory or CWD,
// returning path unmodified on failure.
func (cb *compilationBuilder) tryMakeRelative(path string) string {
	abs, err := filepath.Abs(path)
	if err != nil {
		return path
	}
	var dir string
	if cb.unit.WorkingDirectory != "" {
		dir = cb.unit.WorkingDirectory
	} else {
		dir, err = filepath.Abs(".")
		if err != nil {
			return path
		}
	}
	rel, err := filepath.Rel(dir, abs)
	if err != nil {
		return path
	}
	return rel

}
