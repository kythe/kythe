// Binary parse_test_output parses the `go test -test.v` output written to
// os.Stdin and prints a basic JSON representation of the test results.
package main

import (
	"bufio"
	"encoding/json"
	"encoding/xml"
	"flag"
	"io"
	"log"
	"os"
	"regexp"
	"strings"
	"time"

	"tools/go/testsuite"

	"golang.org/x/tools/benchmark/parse"
)

type status string

// Test case result kinds
const (
	Pass status = "PASS"
	Fail        = "FAIL"
	Skip        = "SKIP"
)

type testCase struct {
	Name     string
	Duration time.Duration `json:",omitempty"`
	Status   status

	Log []string `json:",omitempty"`
}

const logPrefix = "\t"

var testCaseRE = regexp.MustCompile(`^--- (PASS|FAIL|SKIP): ([^ ]+) \((.+)\)$`)

var (
	outputFile = flag.String("out", "/dev/stdout", "Path of file to write test results as XML/JSON")
	format     = flag.String("format", "json", "Format to write results (supported: json or xml)")
)

func main() {
	flag.Parse()
	if *format != "xml" && *format != "json" {
		log.Fatalf("Unsupported output format: %q", *format)
	}

	r, w, err := os.Pipe()
	if err != nil {
		log.Fatal(err)
	}

	go func() {
		if _, err := io.Copy(io.MultiWriter(w, os.Stderr), os.Stdin); err != nil {
			log.Fatal(err)
		}
		if err := w.Close(); err != nil {
			log.Fatal(err)
		}
	}()

	var (
		results  []*testCase
		lastTest *testCase

		benchmarks []*parse.Benchmark
	)

	s := bufio.NewScanner(r)
	for s.Scan() {
		if err := s.Err(); err == io.EOF {
			break
		} else if err != nil {
			log.Fatal(err)
		}

		ss := testCaseRE.FindStringSubmatch(s.Text())
		if len(ss) > 0 {
			if fixed := strings.TrimSuffix(ss[3], " seconds"); fixed != ss[3] {
				// Before
				// https://github.com/golang/go/commit/0e92b538a9f6e337b8e48f47f38803e8245c03cc,
				// this duration format wasn't compatible with time.ParseDuration.  This
				// works around the difference.
				ss[3] = fixed + "s"
			}
			dur, err := time.ParseDuration(ss[3])
			if err != nil {
				log.Fatal(err)
			}
			lastTest = &testCase{
				Name:     ss[2],
				Duration: dur,
				Status:   status(ss[1]),
			}
			results = append(results, lastTest)
		} else if lastTest != nil && strings.HasPrefix(s.Text(), logPrefix) {
			lastTest.Log = append(lastTest.Log, strings.TrimPrefix(s.Text(), logPrefix))
		} else {
			lastTest = nil

			if strings.HasPrefix(s.Text(), "Benchmark") {
				b, err := parse.ParseLine(s.Text())
				if err != nil {
					log.Fatal(err)
				}
				benchmarks = append(benchmarks, b)
			}
		}
	}

	f, err := os.Create(*outputFile)
	if err != nil {
		log.Fatalf("Failure to create output file %q: %v", *outputFile, err)
	}

	switch *format {
	case "xml":
		if err := xml.NewEncoder(f).EncodeElement(makeTestSuite(results, benchmarks), xml.StartElement{
			Name: xml.Name{Local: "testsuite"},
		}); err != nil {
			log.Fatalf("Error encoding XML: %v", err)
		}
	case "json":
		if err := json.NewEncoder(f).Encode(struct {
			Tests      []*testCase
			Benchmarks []*parse.Benchmark `json:",omitempty"`
		}{results, benchmarks}); err != nil {
			log.Fatalf("Error encoding JSON: %v", err)
		}
	default:
		panic("unsupported output format: " + *format)
	}

	if err := f.Close(); err != nil {
		log.Fatalf("Error closing output file %q: %v", *outputFile, err)
	}
}

func makeTestSuite(tests []*testCase, benchmarks []*parse.Benchmark) *testsuite.Suite {
	s := &testsuite.Suite{}
	var totalTime time.Duration
	for _, t := range tests {
		totalTime += t.Duration
		c := testsuite.TestCase{
			Name: t.Name,
			Time: durationToMillis(t.Duration),
		}
		switch t.Status {
		case Skip:
			c.Status = testsuite.NotRun
		case Pass:
			c.Status = testsuite.Run
		case Fail:
			c.Status = testsuite.Run
			c.Errors = append(c.Errors, testsuite.TestError{
				Content: strings.Join(t.Log, "\n"),
			})
		default:
			log.Printf("Unknown test case status: %q", t.Status)
		}
		s.TestCase = append(s.TestCase, c)
	}
	s.Time = durationToMillis(totalTime)
	return s
}

func durationToMillis(d time.Duration) float64 {
	return float64(d.Nanoseconds()) / float64(time.Millisecond)
}
