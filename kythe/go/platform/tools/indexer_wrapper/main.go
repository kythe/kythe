// Binary indexer_wrapper reads compilation units from the provided kzip(s) and
// indexes them using language-specific indexers. Entry protos are written to
// stdout.
//
// TODO(https://github.com/kythe/kythe/pull/3339): after the analyzer driver
// interface is finalized, update this binary to use it rather than creating a
// temporary kzip file for each indexer invocation.
package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/analysis/driver"
	"kythe.io/kythe/go/platform/analysis/local"
	"kythe.io/kythe/go/platform/delimited"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

// delegatingAnalyzer provides an implementation of the CompilationAnalyzer
// interface that forwards to a set of language-specific analyzers.
type delegatingAnalyzer struct {
	analyzers map[string]analysis.CompilationAnalyzer
}

// binPath returns the absolute path to an indexer given its name.
func binPath(binname string) string {
	return filepath.Join(*releaseDir, "indexers", binname)
}

func NewDelegatingAnalyzer(fetcher analysis.Fetcher) (*delegatingAnalyzer, error) {
	// Command-line invocations for known/released indexer binaries.
	cmds := map[string][]string{
		"c++":       []string{binPath("cxx_indexer")},
		"java":      []string{"java", "-jar", binPath("java_indexer.jar")},
		"jvm":       []string{"java", "-jar", binPath("jvm_indexer.jar")},
		"protobuf":  []string{binPath("proto_indexer"), "--index_file"},
		"textproto": []string{binPath("textproto_indexer"), "--index_file"},
		"go":        []string{binPath("go_indexer")},
	}

	analyzers := map[string]analysis.CompilationAnalyzer{}
	for lang, cmd := range cmds {
		var err error
		analyzers[lang], err = local.NewLocalAnalyzer(cmd, fetcher)
		if err != nil {
			return nil, err
		}
	}

	return &delegatingAnalyzer{analyzers: analyzers}, nil
}

func (a *delegatingAnalyzer) Analyze(ctx context.Context, req *apb.AnalysisRequest, f analysis.OutputFunc) error {
	lang := req.Compilation.VName.Language
	analyzer, ok := a.analyzers[lang]
	if !ok {
		return fmt.Errorf("unrecognized language: %q", lang)
	}
	log.Printf("analyzing compilation: {%+v}", req.Compilation.VName)

	return analyzer.Analyze(ctx, req, f)
}

type driverContext struct{}

func (c *driverContext) Setup(ctx context.Context, comp driver.Compilation) error {
	return nil
}
func (c *driverContext) Teardown(ctx context.Context, comp driver.Compilation) error {
	return nil
}

// log any errors that happen during analysis.
func (c *driverContext) AnalysisError(ctx context.Context, comp driver.Compilation, err error) error {
	log.Printf("AnalysisError: %v", err)
	return err
}

var (
	releaseDir = flag.String("release_dir", "/opt/kythe", "Path to kythe release dir, which contains indexer binaries and more")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s file.kzip...

Indexes all compilation units from the provided kzip(s) by delegating to
language-specific indexers. Entry protos are written to stdout.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()
	if flag.NArg() == 0 {
		flag.Usage()
		os.Exit(1)
	}

	// Entry protos are written to stdout as a delimited stream.
	dwr := delimited.NewWriter(os.Stdout)
	writeOutput := func(ctx context.Context, out *apb.AnalysisOutput) error {
		if len(out.Value) > 0 {
			dwr.Put(out.Value)
		}
		return nil
	}

	q := local.NewFileQueue(flag.Args(), nil)
	a, err := NewDelegatingAnalyzer(q)
	if err != nil {
		log.Fatalf("unable to initialize delegating analyzer: %v", err)
	}

	driver := driver.Driver{
		Analyzer:    a,
		Context:     &driverContext{},
		WriteOutput: writeOutput,
	}
	if err := driver.Run(context.Background(), q); err != nil {
		log.Fatalf("analysis failed: %v", err)
	}
}
