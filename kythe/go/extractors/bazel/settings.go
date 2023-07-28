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

package bazel

import (
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"regexp"

	"kythe.io/kythe/go/util/datasize"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/vnameutil"
	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
)

// Settings control the construction of an extractor config from common path
// and name filtering settings.
type Settings struct {
	Corpus      string // the corpus label to assign
	Language    string // the language label to assign (required)
	ExtraAction string // path of blaze.ExtraActionInfo file (required)
	VNameRules  string // path of vnames.json file (optional)
	Include     string // include files matching this RE2 ("" includes all files)
	Exclude     string // exclude files matching this RE2 ("" excludes no files)
	SourceFiles string // mark files matching this RE2 as sources ("" marks none)
	SourceArgs  string // mark arguments matching this RE2 as sources ("" marks none)

	Scoped  bool          // only match source paths within the target package
	Verbose bool          // enable verbose per-file logging
	MaxSize datasize.Size // maximum file size (0 marks no maximum)
}

// SetFlags adds flags to f for each of the fields of s.  The specified prefix
// is prepended to each base flag name. If f == nil, flag.CommandLine is used.
//
// The returned function provides a default usage message that can be used to
// populate flag.Usage or discarded at the caller's discretion.
func (s *Settings) SetFlags(f *flag.FlagSet, prefix string) func() {
	if f == nil {
		f = flag.CommandLine
	}
	p := func(s string) string { return prefix + s }
	f.StringVar(&s.Corpus, p("corpus"), "", "Corpus label to assign (required)")
	f.StringVar(&s.Language, p("language"), "", "Language label to assign (required)")
	f.StringVar(&s.ExtraAction, p("extra_action"), "",
		"Path of blaze.ExtraActionInfo file (required)")
	f.StringVar(&s.VNameRules, p("rules"), "",
		"Path of vnames.json file (optional)")
	f.StringVar(&s.Include, p("include"), "",
		`RE2 matching files to include (if "", include all files)`)
	f.StringVar(&s.Exclude, p("exclude"), "",
		`RE2 matching files to exclude (if "", exclude none)`)
	f.StringVar(&s.SourceFiles, p("source"), "",
		`RE2 matching source files (if "", select none)`)
	f.StringVar(&s.SourceArgs, p("args"), "",
		`RE2 matching arguments to consider source files (if "", select none)`)
	f.BoolVar(&s.Scoped, p("scoped"), false,
		"Only match source paths within the target package")
	f.BoolVar(&s.Verbose, p("verbose"), false,
		"Enable verbose (per-file) logging")
	datasize.FlagVar(f, &s.MaxSize, p("max_file_size"), 0,
		"Maximum size of files to include (if 0, include all files)")

	// A default usage message the caller may use to populate flag.Usage.
	return func() {
		fmt.Fprintf(f.Output(),
			`Usage: %s [options] -corpus C -language L -extra_action f.xa -output f.kzip

Read an ExtraActionInput protobuf message from the designated -extra_action
file[*] and generate a Kythe compilation record in the specified -output file.
The -corpus and -language labels are required to be non-empty.  This only works
for "spawn" actions; any other action will cause the extraction to fail.

By default, all input files listed in the XA are captured in the output.
To capture only files matching a RE2 regexp, use -include.
To explicitly exclude otherwise-matched files, use -exclude.
If both are given, -exclude applies to files selected by -include.

To designate paths as source inputs, set -source to a RE2 regexp matching them,
or set -args to a RE2 regexp matching command-line arguments that should be
considered source paths.  At least one of these must be set, and it is safe to
set both; the results will be merged.

[*] https://bazel.build/versions/master/docs/be/extra-actions.html

Options:
`, filepath.Base(os.Args[0]))
		f.PrintDefaults()
	}
}

func (s *Settings) validate() error {
	switch {
	case s.Language == "":
		return errors.New("you must provide a non-empty language label")
	case s.ExtraAction == "":
		return errors.New("you must provide a non-empty extra action filename")
	case s.SourceFiles == "" && s.SourceArgs == "":
		return errors.New("you must set an expression for source files or source args")
	}
	return nil
}

// NewFromSettings constructs a new *Config from the given settings, and
// returns the extra action info fetched from the corresponding file.
//
// An error is reported if the settings are invalid, or the inputs to the
// config could not be loaded.
func NewFromSettings(s Settings) (*Config, *xapb.ExtraActionInfo, error) {
	if err := s.validate(); err != nil {
		return nil, nil, err
	}

	// Ensure the extra action can be loaded.
	info, err := LoadAction(s.ExtraAction)
	if err != nil {
		return nil, nil, fmt.Errorf("loading extra action: %v", err)
	}
	pkg := PackageName(info.GetOwner())
	log.Infof("Extra action for target %q (package %q)", info.GetOwner(), pkg)

	rules, err := vnameutil.LoadRules(s.VNameRules)
	if err != nil {
		return nil, nil, fmt.Errorf("loading rules: %v", err)
	}

	config := &Config{
		Corpus:     s.Corpus,
		Language:   s.Language,
		Rules:      rules,
		Verbose:    s.Verbose,
		CheckInput: func(path string) (string, bool) { return path, true },
	}

	// If there is a file inclusion regexp, replace the input filter with it.
	if s.Include != "" {
		r, err := regexp.Compile(s.Include)
		if err != nil {
			return nil, nil, fmt.Errorf("invalid inclusion regexp: %v", err)
		}
		config.CheckInput = func(path string) (string, bool) {
			return path, r.MatchString(path)
		}
	}

	// If there is a file exclusion regexp, add it to the input filter.  We
	// know there is already a callback in place (either a match or the trivial
	// default).
	if s.Exclude != "" {
		r, err := regexp.Compile(s.Exclude)
		if err != nil {
			return nil, nil, fmt.Errorf("invalid exclusion regexp: %v", err)
		}

		base := config.CheckInput
		config.CheckInput = func(path string) (string, bool) {
			if path, ok := base(path); ok {
				return path, !r.MatchString(path)
			}
			return "", false
		}
	}

	// If there is a maximum file size, add it to the input filter.
	if s.MaxSize != 0 {
		base := config.CheckInput
		config.CheckInput = func(path string) (string, bool) {
			if path, ok := base(path); ok {
				return path, CheckFileSize(path, s.MaxSize)
			}
			return "", false
		}
	}

	// If there is a source matching regexp, set a filter for it.
	if s.SourceFiles != "" {
		r, err := regexp.Compile(s.SourceFiles)
		if err != nil {
			return nil, nil, fmt.Errorf("invalid source file regexp: %v", err)
		}
		if s.Scoped {
			config.IsSource = func(path string) bool {
				return r.MatchString(path) && PathInPackage(path, pkg)
			}
		} else {
			config.IsSource = r.MatchString
		}
	}

	// If we have been asked to include source files from the argument list,
	// add a fixup to do that at the end.
	if s.SourceArgs != "" {
		r, err := regexp.Compile(s.SourceArgs)
		if err != nil {
			return nil, nil, fmt.Errorf("invalid source args regexp: %v", err)
		}
		config.FixUnit = FindSourceArgs(r)
	}

	return config, info, nil
}
