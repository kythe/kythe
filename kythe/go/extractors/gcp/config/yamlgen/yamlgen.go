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

// Binary yamlgen generates a Cloud Build yaml config file for extracting a
// repo using Kythe.
package main

import (
	"flag"
	"io/ioutil"
	"log"

	"kythe.io/kythe/go/extractors/gcp/config"
)

var (
	input  = flag.String("input", "", "The input config proto to read.")
	output = flag.String("output", "", "The output yaml file to write.")
)

func main() {
	flag.Parse()
	verifyFlags()

	yamlData, err := config.KytheToYAML(*input)
	if err != nil {
		log.Fatalf("failure converting %s to %s: %v", *input, *output, err)
	}

	if err := ioutil.WriteFile(*output, yamlData, 0644); err != nil {
		log.Fatalf("failure writing data to %s: %v", *output, err)
	}
}

func verifyFlags() {
	failed := false
	if *input == "" {
		log.Println("Must specify --input on commandline")
		failed = true
	}
	if *output == "" {
		log.Println("Must specify --output on commandline")
		failed = true
	}
	if failed {
		log.Fatalln("Flags not set properly.")
	}
}
