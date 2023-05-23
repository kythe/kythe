/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

// Program test_vname_rules verifies that a set of vname mapping rules behave
// as expected.
package main

import (
	"encoding/json"
	"flag"
	"io/ioutil"

	"kythe.io/kythe/go/util/log"

	"google.golang.org/protobuf/proto"

	"kythe.io/kythe/go/util/vnameutil"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var (
	rulesFile = flag.String("rules", "", "Path to rules file (required)")
	testsFile = flag.String("tests", "", "Path to tests file (required)")

	numErrs int
)

type test struct {
	Input string `json:"input"`
	Match bool   `json:"match"`
	Want  *struct {
		Corpus string `json:"corpus"`
		Root   string `json:"root"`
		Path   string `json:"path"`
	} `json:"want"`
}

func (t test) vname() *spb.VName {
	if t.Want == nil {
		return nil
	}
	return &spb.VName{
		Corpus: t.Want.Corpus,
		Root:   t.Want.Root,
		Path:   t.Want.Path,
	}
}

func errorf(msg string, args ...any) {
	numErrs++
	log.Errorf(msg, args...)
}

func main() {
	flag.Parse()
	if *rulesFile == "" {
		log.Fatal("You must provide a --rules file to read")
	} else if *testsFile == "" {
		log.Fatal("You must provide a --tests file to read")
	}

	ruleData, err := ioutil.ReadFile(*rulesFile)
	if err != nil {
		log.Fatalf("Error reading rules: %v", err)
	}
	testData, err := ioutil.ReadFile(*testsFile)
	if err != nil {
		log.Fatalf("Error reading tests; %v", err)
	}

	rules, err := vnameutil.ParseRules(ruleData)
	if err != nil {
		log.Fatalf("Error parsing rules: %v", err)
	}
	var testCases []test
	if err := json.Unmarshal(testData, &testCases); err != nil {
		log.Fatalf("Error parsing tests: %v", err)
	}

	for _, test := range testCases {
		got, ok := rules.Apply(test.Input)
		if ok != test.Match {
			errorf("Apply(%q): match=%v, want %v", test.Input, ok, test.Match)
		}
		if want := test.vname(); !proto.Equal(got, want) {
			errorf("Apply(%q):\n got %+v\nwant %+v", test.Input, got, want)
		}
	}
	if numErrs > 0 {
		log.Fatalf("Test failed; %d errors were logged", numErrs)
	}
}
