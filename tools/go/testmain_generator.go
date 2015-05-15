// Binary go_testmain_generator generates a main function for testing
// packages. This is a retrofit of the existing "go test" command for Bazel.
package main

import (
	"bytes"
	"errors"
	"flag"
	"fmt"
	"go/ast"
	"go/doc"
	"go/parser"
	"go/scanner"
	"go/token"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"text/template"
	"unicode"
	"unicode/utf8"
)

var cwd, _ = os.Getwd()

var testFileSet = token.NewFileSet()

type testFuncs struct {
	Tests      []testFunc
	Benchmarks []testFunc
	Examples   []testFunc
	ImportFile string
}

type testFunc struct {
	Package string // imported package name
	Name    string // function name
	Output  string // output, for examples
}

func usage() {
	fmt.Fprintf(os.Stderr, "Usage: %s [options] importfile outputfile pkgfile...\n", os.Args[0])
	flag.PrintDefaults()
	os.Exit(2)
}

func init() {
	flag.Usage = usage
}

func main() {
	flag.Parse()
	if flag.NArg() < 3 {
		flag.Usage()
	}

	importFile := flag.Arg(0)
	outputFile := flag.Arg(1)
	pkgFiles := flag.Args()[2:]

	if err := writeTestmain(importFile, outputFile, pkgFiles); err != nil {
		fmt.Fprintf(os.Stderr, "%s: %v: %s\n", os.Args[0], flag.Args(), err)
		os.Exit(2)
	}
}

// Copied from src/cmd/go/test.go
func isTest(name, prefix string) bool {
	if !strings.HasPrefix(name, prefix) {
		return false
	}
	if len(name) == len(prefix) { // "Test" is ok
		return true
	}
	rune, _ := utf8.DecodeRuneInString(name[len(prefix):])
	return !unicode.IsLower(rune)
}

// Copied from src/cmd/go/build.go
func shortPath(path string) string {
	if rel, err := filepath.Rel(cwd, path); err == nil && len(rel) < len(path) {
		return rel
	}
	return path
}

// Copied from src/cmd/go/pkg.go
func expandScanner(err error) error {
	if err, ok := err.(scanner.ErrorList); ok {
		var buf bytes.Buffer
		for _, e := range err {
			e.Pos.Filename = shortPath(e.Pos.Filename)
			buf.WriteString("\n")
			buf.WriteString(e.Error())
		}
		return errors.New(buf.String())
	}
	return err
}

func writeTestmain(importFile, outputFile string, pkgFiles []string) error {
	t := &testFuncs{ImportFile: importFile}

	for _, pkgFile := range pkgFiles {
		if err := t.load(pkgFile); err != nil {
			return err
		}
	}

	if len(t.Tests) == 0 && len(t.Benchmarks) == 0 && len(t.Examples) == 0 {
		return errors.New("no tests/benchmarks/examples found")
	}

	f, err := os.Create(outputFile)
	if err != nil {
		return err
	}
	defer f.Close()

	if err := testmainTmpl.Execute(f, t); err != nil {
		return err
	}

	return nil
}

func (t *testFuncs) load(filename string) error {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		return err
	}
	f, err := parser.ParseFile(testFileSet, filename, content, parser.ParseComments)
	if err != nil {
		return expandScanner(err)
	}
	pkg := f.Name.String()
	for _, d := range f.Decls {
		n, ok := d.(*ast.FuncDecl)
		if !ok {
			continue
		}
		if n.Recv != nil {
			continue
		}
		name := n.Name.String()
		switch {
		case isTest(name, "Test"):
			t.Tests = append(t.Tests, testFunc{pkg, name, ""})
		case isTest(name, "Benchmark"):
			t.Benchmarks = append(t.Benchmarks, testFunc{pkg, name, ""})
		}
	}
	for _, e := range doc.Examples(f) {
		if e.Output == "" {
			// Don't run examples with no output.
			continue
		}
		t.Examples = append(t.Examples, testFunc{pkg, "Example" + e.Name, e.Output})
	}
	return nil
}

var testmainTmpl = template.Must(template.New("main").Parse(`
package main

import (
	"regexp"
	"testing"

	_test {{.ImportFile | printf "%q"}}
)

var tests = []testing.InternalTest{
{{range .Tests}}
	{"{{.Name}}", _test.{{.Name}}},
{{end}}
}

var benchmarks = []testing.InternalBenchmark{
{{range .Benchmarks}}
	{"{{.Name}}", _test.{{.Name}}},
{{end}}
}

var examples = []testing.InternalExample{
{{range .Examples}}
	{"{{.Name}}", _test.{{.Name}}, {{.Output | printf "%q"}}},
{{end}}
}

var matchPat string
var matchRe *regexp.Regexp

func matchString(pat, str string) (result bool, err error) {
	if matchRe == nil || matchPat != pat {
		matchPat = pat
		matchRe, err = regexp.Compile(matchPat)
		if err != nil {
			return
		}
	}
	return matchRe.MatchString(str), nil
}

func main() {
	testing.Main(matchString, tests, benchmarks, examples)
}
`))
