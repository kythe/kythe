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

	"kythe.io/kythe/go/localrun"
)

var (
	languages         languageFlag = allLanguages()
	port              int
	workingDir        string
	kytheRelease      string
	outputDir         string
	acceptedLanguages = []string{"go", "java"}
)

var notEnoughArgsErr = fmt.Errorf("not enough arguments")

func getTargets(args []string) ([]string, error) {
	if len(args) < 1 {
		// TODO: In go 1.13, replace with this
		// return []string{}, xerrors.Errorf("You provided %d arguments but expected 2: %w", )notEnoughArgsErr
		return []string{}, notEnoughArgsErr
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

Build everything:

	localrun //...

Build just one package:

	localrun //just/this/path/...

Build just go code for one path:

	localrun --language=go //just/this/path/...

Build just go and java code:

	localrun --language=go,java //...
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
	flag.Parse()

	targets, err := getTargets(flag.Args())
	if err != nil {
		usage()
		log.Fatalf("Error invoking localrun: %v\n", err)
	}

	log.Printf("Building %v for targets: %s\n",
		languagesAsString(languages), strings.Join(targets, " "))

	r := &localrun.Runner{
		KytheRelease: "/opt/kythe",
		WorkingDir:   workingDir,
		OutputDir:    outputDir,

		WorkerPoolSize: runtime.GOMAXPROCS(0),

		Languages: languages,
		Targets:   targets,

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

	if err = r.Postprocess(ctx); err != nil {
		log.Fatalf("Error post processing: %v\n", err)
	}

	if err = r.Serve(ctx); err != nil {
		log.Fatalf("Error starting server: %v\n", err)
	}

	log.Println("Finished successfully.")
}

// languageFlag is a flag.Value that accepts either a list of languages or repeated
// languages for the same flag value.
type languageFlag []localrun.Language

// String implements flag.Value.
func (i *languageFlag) String() string {
	s := []string{}
	for _, v := range *i {
		s = append(s, v.String())
	}

	return strings.Join(s, ", ")
}

// Set implements flag.Value.
func (i *languageFlag) Set(value string) error {
	for _, v := range strings.Split(value, ",") {
		l, ok := localrun.LanguageMap[v]
		if !ok {
			return fmt.Errorf("the provided language %q is not one of %s", v, languagesAsString(allLanguages()))
		}

		if i.has(l) {
			continue
		}
		*i = append(*i, l)
		continue
	}
	return nil
}

// has allows you to check if the flag already has a value set.
func (i *languageFlag) has(l localrun.Language) bool {
	for _, v := range *i {
		if v == l {
			return true
		}
	}

	return false
}

func languagesAsString(l []localrun.Language) string {
	languages := []string{}
	for _, l := range l {
		languages = append(languages, l.String())
	}
	return strings.Join(languages, ",")
}

func allLanguages() []localrun.Language {
	languages := []localrun.Language{}
	for c := localrun.Language(0); c.Valid(); c++ {
		languages = append(languages, c)
	}
	return languages
}
