/*
 * Copyright 2018 Google Inc. All rights reserved.
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

// Binary parseconfig provides a tool to parse a configuration file the
// schema defined in kythe.proto.ExtractionConfiguration. Its output is a
// Dockerfile defining an image which can be used to perform an extraction on a
// repository according to the given configuration.
//
// Usage:
//   parseconfig -config <config_file_path> -output <output_file_path>
package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/extractors/config/parser"

	"github.com/golang/protobuf/jsonpb"

	ecpb "kythe.io/kythe/proto/extraction_config_go_proto"
)

var (
	configPath = flag.String("config", "", "Path for the JSON extraction configuration file")
	outputPath = flag.String("output", "", "Path for output Dockerfile")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s -config <path> -output <path>

Parses the specified extraction configuration file given by -config. Uses
the parsed data to construct a customized Docker image which can be used
to extract the configuration's corresonding repository. Writes the generated
image to a Dockerfile written to -output.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func verifyFlags() {
	if len(flag.Args()) > 0 {
		log.Fatalf("Unknown arguments: %v", flag.Args())
	}

	switch {
	case *configPath == "":
		log.Fatal("You must provide a non-empty -config")
	case *outputPath == "":
		log.Fatal("You must provide a non-empty -output")
	}
}

func main() {
	flag.Parse()
	verifyFlags()

	// open the specified config file
	configFile, err := os.Open(*configPath)
	if err != nil {
		log.Fatalf("Error opening -config: %v", err)
	}
	defer configFile.Close()

	// attempt to deserialize the extraction config
	config := ecpb.ExtractionConfiguration{}
	if err := jsonpb.Unmarshal(configFile, &config); err != nil {
		log.Fatalf("Error parsing -config: %v", err)
	}

	// attempt to generate a docker image from the specified config
	image, err := parser.NewExtractionImage(&config)
	if err != nil {
		log.Fatalf("Error generating extraction image: %v", err)
	}

	// write the generated extraction image to the specified output file
	if err = ioutil.WriteFile(*outputPath, image, 0644); err != nil {
		log.Fatalf("Error writing -output: %v", err)
	}
}
