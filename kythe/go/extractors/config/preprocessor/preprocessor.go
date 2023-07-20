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

// Binary preprocessor can modify a repo's build configuration file with the
// necessary bits to run Kythe extraction.
//
// For detailed usage, see the README.md file.
package main

import (
	"flag"
	"fmt"
	"strings"

	"kythe.io/kythe/go/extractors/config/preprocessor/modifier"
	"kythe.io/kythe/go/extractors/constants"
	"kythe.io/kythe/go/util/log"
)

var (
	forcedBuilder = flag.String("builder", "", "To force preprocessor to interpret its input file, specify -builder=gradle or -builder=mvn")
	javac         = flag.String("javac_wrapper", "", "A wrapper script around javac that runs kythe extraction.  Some builders may require this to go directly into the build file itself.")
)

func preProcess(buildFile string) error {
	builder, err := findBuilder(buildFile)
	if err != nil {
		return err
	}

	switch builder {
	case "gradle":
		return modifier.PreProcessBuildGradle(buildFile, javacWrapper())
	case "mvn":
		return modifier.PreProcessPomXML(buildFile)
	default:
		return fmt.Errorf("unsupported builder type: %s", builder)
	}
}

func findBuilder(buildFile string) (string, error) {
	if *forcedBuilder != "" {
		return *forcedBuilder, nil
	}

	if strings.HasSuffix(buildFile, "build.gradle") {
		return "gradle", nil
	}
	if strings.HasSuffix(buildFile, "pom.xml") {
		return "mvn", nil
	}
	return "", fmt.Errorf("unrecognized or unsupported build file: %s", buildFile)
}

func javacWrapper() string {
	if *javac != "" {
		return *javac
	}
	return constants.DefaultJavacWrapperLocation
}

func main() {
	flag.Parse()
	args := flag.Args()

	if len(args) != 1 {
		log.Fatal("Must specify a build file via ./preprocessor <build-file>")
	}

	err := preProcess(args[0])
	if err != nil {
		log.Fatal(err)
	}
}
