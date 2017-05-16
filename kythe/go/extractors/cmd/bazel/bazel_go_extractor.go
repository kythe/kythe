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

// bazel_go_extractor is a Bazel extra action that extracts Go compilations.
package main

import (
	"context"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"

	"bitbucket.org/creachadair/shell"
	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"

	"kythe.io/kythe/go/extractors/bazel"
	"kythe.io/kythe/go/extractors/govname"
	"kythe.io/kythe/go/platform/kindex"
	"kythe.io/kythe/go/util/vnameutil"

	gopb "kythe.io/kythe/proto/go_proto"
	spb "kythe.io/kythe/proto/storage_proto"
	eapb "kythe.io/third_party/bazel/extra_actions_base_proto"
)

var (
	corpus = flag.String("corpus", "kythe", "The corpus label to assign (required)")
)

const baseUsage = `Usage: %[1]s [flags] <extra-action> <output-file> <vname-config>`

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, baseUsage+`

Extract a Kythe compilation record for Go from a Bazel extra action.

Arguments:
 <extra-action> is a file containing a wire format ExtraActionInfo protobuf.
 <output-file>  is the path where the output kindex file is written.
 <vname-config> is the path of a VName configuration JSON file.

Flags:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()
	if flag.NArg() != 3 {
		log.Fatalf(baseUsage+` [run "%[1]s --help" for details]`, filepath.Base(os.Args[0]))
	}
	extraActionFile := flag.Arg(0)
	outputFile := flag.Arg(1)
	vnameRuleFile := flag.Arg(2)

	info, err := loadExtraAction(extraActionFile)
	if err != nil {
		log.Fatalf("Error loading extra action: %v", err)
	}
	if m := info.GetMnemonic(); m != "GoCompile" {
		log.Fatalf("Extractor is not applicable to this action: %q", m)
	}

	// Load vname rewriting rules. We handle this directly, becaues the Bazel
	// Go rules have some pathological symlink handling that the normal rules
	// need to be patched for.
	rules, err := loadRules(vnameRuleFile)
	if err != nil {
		log.Fatalf("Error loading vname rules: %v", err)
	}

	ext := &extractor{rules: rules}
	config := &bazel.Config{
		Corpus:      *corpus,
		Language:    govname.Language,
		CheckAction: ext.checkAction,
		CheckInput:  ext.checkInput,
		CheckEnv:    ext.checkEnv,
		Fixup:       ext.fixup,
	}
	cu, err := config.Extract(context.Background(), info)
	if err != nil {
		log.Fatalf("Extraction failed: %v", err)
	}

	// Write and flush the output to a .kindex file.
	if err := writeToFile(cu, outputFile); err != nil {
		log.Fatalf("Writing output failed: %v", err)
	}
}

// writeToFile creates the specified output file from the contents of w.
func writeToFile(w io.WriterTo, path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	_, err = w.WriteTo(f)
	cerr := f.Close()
	if err != nil {
		return err
	}
	return cerr
}

// loadExtraAction reads the contents of the file at path and decodes it as an
// ExtraActionInfo protobuf message.
func loadExtraAction(path string) (*eapb.ExtraActionInfo, error) {
	bits, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var info eapb.ExtraActionInfo
	if err := proto.Unmarshal(bits, &info); err != nil {
		return nil, err
	}
	return &info, nil
}

// loadRules reads the contents of the file at path and decodes it as a
// slice of vname rewriting rules. The result is empty if path == "".
func loadRules(path string) (vnameutil.Rules, error) {
	if path == "" {
		return nil, nil
	}
	bits, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	return vnameutil.ParseRules(bits)
}

type extractor struct {
	rules        vnameutil.Rules
	toolArgs     *toolArgs
	goos, goarch string
}

func (e *extractor) checkAction(_ context.Context, info *eapb.SpawnInfo) error {
	toolArgs, err := extractToolArgs(info.Argument)
	if err != nil {
		return fmt.Errorf("extracting tool arguments: %v", err)
	}
	e.toolArgs = toolArgs

	for _, evar := range info.Variable {
		switch name := evar.GetName(); name {
		case "GOOS":
			e.goos = evar.GetValue()
		case "GOARCH":
			e.goarch = evar.GetValue()
		}
	}
	return nil
}

func (e *extractor) checkInput(path string) (string, bool) {
	return path, e.toolArgs.wantInput(path)
}

func (*extractor) checkEnv(name, _ string) bool {
	switch name {
	case "PATH", "GOOS", "GOARCH":
		return false
	}
	return true
}

func (e *extractor) fixup(cu *kindex.Compilation) error {
	unit := cu.Proto
	unit.Argument = e.toolArgs.compile
	unit.SourceFile = e.toolArgs.sources
	unit.WorkingDirectory = e.toolArgs.workDir
	unit.VName.Path = e.toolArgs.importPath

	// Adjust the paths through the symlink forest.
	for i, fi := range cu.Files {
		path := fi.Info.Path
		fixed := e.toolArgs.fixPath(path)
		fi.Info.Path = fixed
		unit.RequiredInput[i].VName = e.rules.ApplyDefault(fixed, &spb.VName{
			Corpus: *corpus,
			Path:   fixed,
		})

	}
	return cu.AddDetails(&gopb.GoDetails{
		Goroot:     e.toolArgs.goRoot,
		Goos:       e.goos,
		Goarch:     e.goarch,
		CgoEnabled: e.toolArgs.useCgo,
	})
}

// toolArgs captures the settings expressed by the Go compiler tool and its
// arguments.
type toolArgs struct {
	compile     []string          // compiler argument list
	paramsFile  string            // the response file, if one was used
	workDir     string            // the compiler's working directory
	goRoot      string            // the GOROOT path
	importPath  string            // the import path being compiled
	includePath string            // an include path, if set
	outputPath  string            // the output from the compiler
	toolRoot    string            // root directory for compiler/libraries
	useCgo      bool              // whether cgo is enabled
	useRace     bool              // whether the race-detector is enabled
	pathmap     map[string]string // a mapping from physical path to expected path
	sources     []string          // source file paths

	// The file paths written by the Go compile actions do not have the names
	// the compiler expects to match with the package import paths. Instead,
	// the action creates a symlink forest where the links have the expected
	// names and the targets of those links are the files as emitted.
	//
	// The pathmap allows us to invert this mapping, so that the files stored
	// in a compilation record have the paths the compiler expects.
}

// fixPath remaps path through the path map if it is present; or otherwise
// returns path unmodified.
func (g *toolArgs) fixPath(path string) string {
	if fixed, ok := g.pathmap[path]; ok {
		trimmed := trimPrefixDir(fixed, g.workDir)
		if root, ok := findBazelOut(path); ok && !strings.Contains(trimmed, root) {
			return filepath.Join(root, trimmed)
		}
		return trimmed
	}
	return path
}

// findBazelOut reports whether path is rooted under a Bazel output directory,
// and if so returns the prefix of the path corresponding to that directory.
func findBazelOut(path string) (string, bool) {
	// Bazel stores outputs from the build process in a directory structure
	// of the form bazel-out/<build-config>/<tag>/..., for example:
	//
	//    bazel-out/local_linux-fastbuild/genfiles/foo/bar.cc
	//
	// We detect this structure by checking for a prefix of the path with three
	// or more components, the first of which is "bazel-out".

	parts := strings.SplitN(path, string(filepath.Separator), 4)
	if len(parts) >= 3 && parts[0] == "bazel-out" {
		return filepath.Join(parts[:3]...), true
	}
	return "", false
}

// wantInput reports whether path should be included as a required input.
func (g *toolArgs) wantInput(path string) bool {
	// Drop the response file (if there is one).
	if path == g.paramsFile {
		return false
	}

	// Otherwise, anything that isn't in the tool root we keep.
	trimmed, err := filepath.Rel(g.toolRoot, path)
	if err != nil || trimmed == path {
		return true
	}

	// Within the tool root, we keep library inputs, but discard binaries.
	// Filter libraries based on the race-detector settings.
	prefix, tail := splitPrefix(trimmed)
	switch prefix {
	case "bin/", "cmd/":
		return false
	case "pkg/":
		sub, _ := splitPrefix(tail)
		if strings.HasSuffix(sub, "_race/") && !g.useRace {
			return false
		}
		return sub != "tool/"
	default:
		return true // conservative fallback
	}
}

// bazelArgs captures compiler settings extracted from a Bazel response file.
type bazelArgs struct {
	paramsFile string            // the path of the params file (if there was one)
	goRoot     string            // the corpus-relative path of the Go root
	workDir    string            // the corpus-relative working directory
	compile    []string          // the compiler argument list
	symlinks   map[string]string // a mapping from original path to linked path

	// TODO(fromberger): See if we can fix the rule definitions to emit the
	// output in the correct format, so the symlink forest isn't needed.
	// See also http://github.com/bazelbuild/rules_go/issues/211.
}

// checkParams reports whether args looks like a response file execution,
// consisting either of a plain .params file or a bash -c command with an
// argument file. If so, the response file path is returned.
func checkParams(args []string) (string, bool) {
	if len(args) == 1 && filepath.Ext(args[0]) == ".params" {
		return args[0], true
	} else if len(args) == 3 && args[0] == "/bin/bash" && args[1] == "-c" {
		return args[2], true
	}
	return "", false
}

// parseBazelArgs extracts the compiler command line from the raw argument list
// passed in by Bazel. The official Go rules currently pass in a response file
// containing a shell script that we have to parse.
func parseBazelArgs(args []string) (*bazelArgs, error) {
	paramsFile, ok := checkParams(args)
	if !ok {
		// This is some unusual case; assume the arguments are already parsed.
		return &bazelArgs{compile: args}, nil
	}

	// This is the expected case, a response file.
	result := &bazelArgs{paramsFile: paramsFile}
	f, err := os.Open(result.paramsFile)
	if err != nil {
		return nil, err
	}
	data, err := ioutil.ReadAll(f)
	f.Close()
	if err != nil {
		return nil, err
	}

	// Split up the response into lines, and split each line into commands
	// assuming a pipeline of the form "cmd1 && cmd2 && ...".
	// Bazel exports GOROOT and changes the working directory, both of which we
	// want for processing the compiler's argument list.
	result.symlinks = make(map[string]string)
	var last, srcs []string
	parseShellCommands(data, func(cmd string, args []string) {
		last = append([]string{cmd}, args...)
		switch cmd {
		case "export":
			if dir := strings.TrimPrefix(args[0], "GOROOT=$(pwd)/"); dir != args[0] {
				result.goRoot = filepath.Clean(dir)
			}
		case "cd":
			result.workDir = args[0]
		case "ln":
			if len(args) == 3 && args[0] == "-s" {
				result.symlinks[args[1]] = args[2]
			}
		default:
			// The source arguments are collected using some bash nonsense.
			// Reverse-engineer this.
			//
			// TODO(fromberger): Figure out a better way to deal with this,
			// and probably file a bug upstream.
			//
			// The current incarnation of the Bazel rules expects to be able to
			// run a filtering tool before the compiler, but it does this
			// buried in the response file and it's tricky for us to replay
			// that because we run without write access to those paths.
			//
			// We could maybe dike out the commands and run the filter, but for
			// now we'll just take all the unfiltered source files as is.
			trimmed := strings.TrimPrefix(cmd, "UNFILTERED_GO_FILES=(")
			if trimmed != cmd {
				line := strings.Join(append([]string{trimmed}, args...), " ")
				srcs, _ = shell.Split(strings.TrimRight(line, ")"))
			}
		}
	})
	const filteredMarker = `${FILTERED_GO_FILES[@]}`
	for _, arg := range last {
		if arg == filteredMarker {
			result.compile = append(result.compile, srcs...)
		} else {
			result.compile = append(result.compile, arg)
		}
	}
	return result, nil
}

// extractToolArgs extracts the build tool arguments from args.
func extractToolArgs(args []string) (*toolArgs, error) {
	parsed, err := parseBazelArgs(args)
	if err != nil {
		return nil, err
	}

	result := &toolArgs{
		paramsFile: parsed.paramsFile,
		workDir:    parsed.workDir,
		goRoot:     filepath.Join(parsed.workDir, parsed.goRoot),
		pathmap:    make(map[string]string),
	}

	// Process the parsed command-line arguments to find the tool, source, and
	// output paths.
	var wantArg *string
	inTool := false
	for _, arg := range parsed.compile {
		// Discard arguments until the tool binary is found.
		if !inTool {
			if filepath.Base(arg) == "go" {
				adjusted := filepath.Join(result.workDir, arg)
				result.toolRoot = filepath.Dir(filepath.Dir(adjusted))
				result.compile = append(result.compile, adjusted)
				inTool = true
			}
			continue
		}

		// Scan for important flags.
		if wantArg != nil { // capture argument for a previous flag
			*wantArg = filepath.Join(parsed.workDir, arg)
			result.compile = append(result.compile, *wantArg)
			wantArg = nil
			continue
		}
		result.compile = append(result.compile, arg)
		if arg == "-p" {
			wantArg = &result.importPath
		} else if arg == "-o" {
			wantArg = &result.outputPath
		} else if arg == "-I" {
			wantArg = &result.includePath
		} else if arg == "-race" {
			result.useRace = true
		} else if !strings.HasPrefix(arg, "-") && strings.HasSuffix(arg, ".go") {
			result.sources = append(result.sources, arg)
		}
	}

	// Reverse-engineer the symlink forest to recover the paths the compiler is
	// expecting to see so the captured inputs map correctly.
	for physical, logical := range parsed.symlinks {
		result.pathmap[cleanLinkTarget(physical)] = logical
	}

	return result, nil
}

// parseShellCommands splits input into lines and parses each line as a shell
// pipeline of the form "cmd1 && cmd2 && ...". Each resulting command and its
// arguments are passed to f in their order of occurrence in the input.
func parseShellCommands(input []byte, f func(cmd string, args []string)) {
	for _, line := range strings.Split(string(input), "\n") {
		words, _ := shell.Split(strings.TrimSpace(line))
		for len(words) > 0 {
			i := stringset.Index("&&", words...)
			if i < 0 {
				f(words[0], words[1:])
				break
			}
			f(words[0], words[1:i])
			words = words[i+1:]
		}
	}
}

// splitPrefix separates the first slash-delimited component of path.
// The prefix includes the slash, so that prefix + tail == path.
// If there is no slash in the path, prefix == "".
func splitPrefix(path string) (prefix, tail string) {
	if i := strings.Index(path, "/"); i >= 0 {
		return path[:i+1], path[i+1:]
	}
	return "", path
}

// cleanLinkTarget removes ".." markers from the head of path, and returns the
// cleaned remainder.
func cleanLinkTarget(path string) string {
	const up = "../"
	for strings.HasPrefix(path, up) {
		path = path[len(up):]
	}
	return filepath.Clean(path)
}

// trimPrefixDir makes path relative to dir is possible; otherwise it returns
// path unmodified.
func trimPrefixDir(path, dir string) string {
	if rel, err := filepath.Rel(dir, path); err == nil {
		return rel
	}
	return path
}
