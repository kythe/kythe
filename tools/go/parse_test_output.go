// Binary parse_test_output parses the `go test -test.v` output written to
// os.Stdin and prints a basic JSON representation of the test results.
package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"io"
	"log"
	"os"
	"regexp"
	"strings"
	"time"

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

func main() {
	flag.Parse()

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

	json.NewEncoder(os.Stdout).Encode(struct {
		Tests      []*testCase
		Benchmarks []*parse.Benchmark `json:",omitempty"`
	}{results, benchmarks})
}
