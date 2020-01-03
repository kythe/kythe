/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	"kythe.io/kythe/go/localrun"
	"kythe.io/kythe/go/util/datasize"

	// Side effect import used to register the handler for gsutil
	_ "kythe.io/kythe/go/storage/leveldb"
)

var (
	languages         languageFlag = languageFlag{localrun.AllLanguages()}
	port              int
	workingDir        string
	kytheRelease      string
	outputDir         string
	indexingTimeout   time.Duration
	acceptedLanguages = []string{"go", "java"}
	cacheSize         = datasize.Flag("cache_size", "3gb", "How much ram to dedicate to handling")
)

var errNotEnoughArgs = fmt.Errorf("not enough arguments")

func getTargets(args []string) ([]string, error) {
	if len(args) < 1 {
		// TODO: In go 1.13, replace with this
		// return []string{}, xerrors.Errorf("You provided %d arguments but expected 2: %w", errNotEnoughArgs)
		return []string{}, errNotEnoughArgs
	}
	return args[:], nil
}

// usage prints out usage information for the program to stderr.
func usage() {
	fmt.Fprintf(os.Stderr, `localrun

localrun is a small binary that can be used to build, extract, index, and serve any bazel repo

Usage:

localrun [--language=a,b,c] [targets+]

Flag arguments:

`)
	flag.Usage()

	fmt.Fprintf(os.Stderr, `
Positional arguments:

  [targets+]: A list of one or more Bazel build targets that should be included in processing.

Sample invocations:

"bazel build" and generate cross references for all languages for everything:

	localrun //...

"bazel build" a portion of the repository and generate cross references for all languages:

	localrun //just/this/path/...

"bazel build" a portion of the repository (including non-go code) and generate cross references
for just the go code:

	localrun --language=go //just/this/path/...

"bazel build" and generate cross references for just go and java code in the whole repo:

	localrun --language=go,java //...

Since this uses a standard bazel build invocation to build the targets, any targets that are
included in the path will be included. If you would like to limit the build to just java_library
targets you can employ bazel query to this end.

Build only java_library targets and cross reference only java code:

bazel query 'kind(java_library, //...)' | xargs localrun
`)
}

// main entrypoint for the program.
func main() {
	wd, _ := os.Getwd()
	cacheDir, _ := os.UserCacheDir()

	flag.Var(&languages, "language", "Case insensitive list of languages that should be extracted and indexed")
	flag.IntVar(&port, "port", 8080, "The port to start the http server on once everything is finished")
	flag.StringVar(&workingDir, "working_dir", wd, "The directory for all bazel oprations to begin relative to")
	flag.StringVar(&kytheRelease, "kythe_release", "/opt/kythe", "The directory that holds a Kythe release. Releases can be downloaded from https://github.com/kythe/kythe/releases")
	flag.StringVar(&outputDir, "output_dir", filepath.Join(cacheDir, "output"), "The directory to create intermediate artifacts in")
	flag.DurationVar(&indexingTimeout, "indexing_timeout", 300*time.Second, "How long to wait before indexing a compilation unit times out")
	flag.Parse()

	targets, err := getTargets(flag.Args())
	if err != nil {
		usage()
		log.Fatalf("Error invoking localrun: %v\n", err)
	}

	log.Printf("Building %v for targets: %s\n",
		languages.LanguageSet.String(), strings.Join(targets, " "))

	r := &localrun.Runner{
		KytheRelease: kytheRelease,
		WorkingDir:   workingDir,
		OutputDir:    outputDir,
		CacheSize:    cacheSize,

		//WorkerPoolSize: runtime.GOMAXPROCS(0) * 2,
		WorkerPoolSize: runtime.GOMAXPROCS(0)*0 + 1,

		Languages: languages.LanguageSet,
		Targets:   targets,

		Timeout: 300 * time.Second,

		Port: port,
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	if err = r.Extract(ctx); err != nil {
		log.Fatalf("Error extracting: %v\n", err)
	}

	if err = r.Index(ctx); err != nil {
		log.Fatalf("Error indexing: %v\n", err)
	}

	if err = r.PostProcess(ctx); err != nil {
		log.Fatalf("Error post processing: %v\n", err)
	}

	if err = r.Serve(ctx); err != nil {
		log.Fatalf("Error starting server: %v\n", err)
	}

	log.Println("Finished successfully.")
}

// languageFlag is a flag.Value that accepts either a list of languages or repeated
// languages for the same flag value.
type languageFlag struct {
	localrun.LanguageSet
}

// String implements flag.Value.
func (lf *languageFlag) String() string {
	return lf.LanguageSet.String()
}

// Set implements flag.Value.
func (lf *languageFlag) Set(value string) error {
	for _, v := range strings.Split(value, ",") {
		l, ok := localrun.LanguageMap[v]
		if !ok {
			return fmt.Errorf("the provided language %q is not one of %s", v, localrun.AllLanguages().String())
		}

		if lf.LanguageSet.Has(l) {
			continue
		}
		lf.LanguageSet.Set(l)
		continue
	}
	return nil
}
