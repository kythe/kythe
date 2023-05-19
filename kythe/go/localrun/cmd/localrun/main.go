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

// Binary localrun is a small binary that can be used to build, extract, index,
// and serve any bazel repo.
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	"kythe.io/kythe/go/localrun"
	"kythe.io/kythe/go/util/datasize"
	"kythe.io/kythe/go/util/log"

	// Side effect import used to register the handler for gsutil
	_ "kythe.io/kythe/go/storage/leveldb"
)

var (
	languages      languageFlag = languageFlag{localrun.AllLanguages()}
	workerPoolSize              = flag.Int("worker_pool_size", 1, "Number of workers to use")
)

func init() {
	flag.Var(&languages, "language", "Case insensitive list of languages that should be extracted and indexed")
}

var (
	hostname        = flag.String("hostname", "localhost", "The host to start the http server on once everything is finished")
	port            = flag.Int("port", 8080, "The port to start the http server on once everything is finished")
	workingDir      = flag.String("working_dir", mustString(os.Getwd), "The directory for all bazel oprations to begin relative to")
	kytheRelease    = flag.String("kythe_release", "/opt/kythe", "The directory that holds a Kythe release. Releases can be downloaded from https://github.com/kythe/kythe/releases")
	publicResources = flag.String("public_resources", "", "Path to the public resources to serve in the webserver (default: $kythe_release/resources/public)")
	outputDir       = flag.String("output_dir", filepath.Join(mustString(os.UserCacheDir), "kythe", "output"), "The directory to create intermediate artifacts in")
	indexingTimeout = flag.Duration("indexing_timeout", 300*time.Second, "How long to wait before indexing a compilation unit times out")
	cacheSize       = datasize.Flag("cache_size", "3gb", "How much ram to dedicate to handling")
)

var errNotEnoughArgs = errors.New("not enough arguments")

func getTargets(args []string) ([]string, error) {
	if len(args) < 1 {
		return nil, fmt.Errorf("You provided %d arguments but expected 2: %w", len(args), errNotEnoughArgs)
	}
	return args, nil
}

// usage prints out usage information for the program to stderr.
func usage() {
	fmt.Fprintf(os.Stderr, `localrun

localrun is a small binary that can be used to build, extract, index, and serve any bazel repo.

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
	flag.Parse()

	if *publicResources == "" {
		*publicResources = *kytheRelease + "/resources/public"
	}

	targets, err := getTargets(flag.Args())
	if err != nil {
		usage()
		log.Fatalf("Error invoking localrun: %v", err)
	}

	log.Infof("Building %v for targets: %s\n",
		languages.LanguageSet.String(), strings.Join(targets, " "))

	if *workerPoolSize <= 0 {
		*workerPoolSize = runtime.GOMAXPROCS(0)
	}

	r := &localrun.Runner{
		KytheRelease: *kytheRelease,
		WorkingDir:   *workingDir,
		OutputDir:    *outputDir,
		CacheSize:    cacheSize,

		WorkerPoolSize: *workerPoolSize,

		Languages: languages.LanguageSet,
		Targets:   targets,

		Timeout: *indexingTimeout,

		Port:            *port,
		Hostname:        *hostname,
		PublicResources: *publicResources,
	}

	// WithCancel used to prevent run-away goroutines from never exiting.
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

	log.Info("Finished successfully.")
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
		l, ok := localrun.KnownLanguage(v)
		if !ok {
			return fmt.Errorf("the provided language %q is not one of %s", v, localrun.AllLanguages().String())
		}

		lf.LanguageSet.Set(l)
	}
	return nil
}

func mustString(f func() (string, error)) string {
	s, err := f()
	if err != nil {
		fmt.Printf("Error getting string: %v", err)
		os.Exit(1)
	}
	return s
}
