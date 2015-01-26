/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Binary kytheextractor creates a compilation index file from a compilation action.
// The information is passed through command line arguments; the extractor
// takes the arguments usually provided to the Go compiler. Additionally,
// the required inputs are specified using the --inputs flag, as comma
// separated values.
// Required inputs are read from the local file system.
// Output index files are written on the local file system, in the output
// directory specified by the KYTHE_OUTPUT_DIRECTORY environment variable.
// If the variable is not defined ,then the current directory is used.
// Usage of this tool is:
// ./kytheextractor --inputs comma,separated,required,inputs [--corpus corpus_name] [space separated compiler arguments]
package main

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"

	"kythe/go/platform/indexinfo"
	"kythe/go/platform/local"
)

const (
	analysisTargetEnv = "KYTHE_ANALYSIS_TARGET"
	corpusEnv         = "KYTHE_CORPUS"
	outputDirEnv      = "KYTHE_OUTPUT_DIRECTORY"

	defaultCorpus = "kythe"
	languageName  = "go"
)

var (
	corpus    = getEnvVar(corpusEnv, defaultCorpus)
	outputDir = getEnvVar(outputDirEnv, ".")
)

// compilationInput stores compilation input information extracted from command line arguments.
type compilationInput struct {
	Inputs []string
	Args   []string
}

func main() {
	cinput, err := parseArguments(os.Args[1:])
	if err != nil {
		fmt.Fprintln(os.Stderr, "Usage: kytheextractor --inputs comma,separated,input,files [--corpus corpus_name] [space separated arguments]")
		log.Panicf("%v", err)
	}

	uLocal, err := cinput.toCompilation()
	if err != nil {
		log.Fatalf("Error creating compilation: %v", err)
	}

	uIndex, err := indexinfo.FromCompilation(uLocal.Proto, uLocal)
	if err != nil {
		log.Fatalf("Error creating index file: %v", err)
	}

	filename := cleanFilename(uLocal.Proto.GetVName().GetSignature()) + ".kindex"

	f, err := os.Create(filepath.Join(outputDir, filename))
	if err != nil {
		log.Fatalf("Error creating output file: %v", err)
	}

	defer func() {
		if err := f.Close(); err != nil {
			log.Printf("Error closing output file: %v", err)
		}
	}()

	if _, err := uIndex.WriteTo(f); err != nil {
		log.Printf("Error writing index: %v", err)
	}
}

// parseArguments parses command line arguments and generates a compilationInput structure.
func parseArguments(args []string) (*compilationInput, error) {

	l := len(args)
	var compilationArgs []string

	switch {
	case l < 2:
		return nil, fmt.Errorf("Expecting at least one argument, provided %v.", len(args))
	case args[0] != "--inputs" && args[0] != "-inputs":
		return nil, fmt.Errorf("Expecting --inputs flag as first argument; provided %q", args[0])
	case l >= 4 && (args[2] == "--corpus" || args[2] == "-corpus"):
		corpus = args[3]
		if l > 4 {
			compilationArgs = args[4:]
		}
	case l > 2:
		compilationArgs = args[2:]
	}

	cinput := compilationInput{
		Inputs: strings.Split(args[1], ","),
		Args:   compilationArgs,
	}

	return &cinput, nil
}

// toCompilation transforms a compilationInput struct to a local.Compilation
// data structure.
func (ci *compilationInput) toCompilation() (*local.Compilation, error) {
	comp := local.NewCompilation()

	comp.SetSignature(ci.extractSignature())
	comp.SetLanguage(languageName)
	comp.SetCorpus(corpus)

	isArg := make(map[string]bool)
	for _, arg := range ci.Args {
		isArg[arg] = true
	}
	// Add required inputs.
	for _, input := range ci.Inputs {
		if err := comp.AddFile(input); err != nil {
			return nil, fmt.Errorf("failed to add input file %q: %v", input, err)
		}
		// If a required input appears in the arguments list, then it is a source file.
		if isArg[input] {
			comp.SetSource(input)
		}
	}

	comp.Proto.Argument = ci.Args

	return comp, nil
}

// extractSignature extracts a signature string from the compilation's inputs or
// reads it from the os environment, if possible
func (ci *compilationInput) extractSignature() string {
	target := os.Getenv(analysisTargetEnv)
	if target != "" {
		return target
	}

	hash := sha256.New()
	for _, input := range ci.Inputs {
		hash.Write([]byte(input))
	}
	md := hash.Sum(nil)
	return hex.EncodeToString(md)
}

func getEnvVar(name, defaultValue string) string {
	v := os.Getenv(name)
	if v == "" {
		return defaultValue
	}
	return v
}

func cleanFilename(name string) string {
	return strings.Replace(strings.Trim(name, "/"), "/", "_", -1)
}
