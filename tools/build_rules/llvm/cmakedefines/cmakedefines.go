/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Processes CMake input files to handle CMake-style substitutions.
//
// For details see:
// https://cmake.org/cmake/help/latest/command/configure_file.html
package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"regexp"
	"strings"
)

type cmakeValue string
type cmakeDefines map[string]cmakeValue

var (
	atOnly       = flag.Bool("at_only", false, "Restrict variable replacement to references of the form @VAR@.")
	strict       = flag.Bool("strict", false, "Exit with an error if an undefined variable is inspected.")
	verbose      = flag.Bool("verbose", false, "Print an error if an undefined variable is inspected.")
	jsonValues   = flag.String("json", "", "Parse the string as a JSON dictionary to use as CMake defines.")
	outFile      = flag.String("outfile", "-", "File to which output should be written. Defaults to stdout.")
	atPattern    = regexp.MustCompile("@([^@]*)@")
	allPattern   = regexp.MustCompile(`\${([^}]*)}|` + atPattern.String())
	defPattern   = regexp.MustCompile(`^#(\s*)cmakedefine(01)?\s+([^\s]+)`)
	falsePattern = regexp.MustCompile("^(0|OFF|NO|FALSE|N|IGNORE|(.*-)?NOTFOUND)$")
	varMap       = &cmakeDefines{}
)

func (cv *cmakeValue) UnmarshalJSON(data []byte) error {
	var i interface{}
	json.Unmarshal(data, &i)
	switch v := i.(type) {
	case bool:
		*cv = cmakeValue(strings.ToUpper(fmt.Sprint(v)))
	case float64:
		*cv = cmakeValue(fmt.Sprint(v))
	case string:
		*cv = cmakeValue(v)
	case nil:
		*cv = ""
	default:
		return fmt.Errorf("expected bool, string or number, found: %T", v)
	}
	return nil
}

// Get returns the currently defined value or the empty string.
func (vars *cmakeDefines) Get(key string) string {
	if value, ok := (*vars)[key]; ok {
		return string(value)
	} else if *strict {
		log.Fatalln("Undefined variable: ", key)
	} else if *verbose {
		log.Println("Undefined variable: ", key)
	}
	return ""
}

// Defined returns true if the value is set to a value "not considered false".
// See https://cmake.org/cmake/help/latest/command/if.html#command:if for more details.
func (vars *cmakeDefines) Defined(key string) bool {
	value, ok := (*vars)[key]
	if !ok {
		if *strict {
			log.Fatalln("Undefined variable: ", key)
		} else if *verbose {
			log.Println("Undefined variable: ", key)
		}
	}
	return ok && len(value) > 0 && !falsePattern.MatchString(string(value))
}

func (vars *cmakeDefines) String() string {
	bytes, err := json.Marshal(vars)
	if err != nil {
		panic(err)
	}
	return string(bytes)
}

func init() {
	flag.Usage = func() {
		fmt.Fprintf(flag.CommandLine.Output(), "Usage: %s [options] <input>\n", os.Args[0])
		flag.PrintDefaults()
	}
}

func transform(input, output *os.File) error {
	scanner := bufio.NewScanner(input)
	for scanner.Scan() {
		text := scanner.Bytes()
		text = replaceLine(text)
		text = append(text, '\n')
		if _, err := output.Write(text); err != nil {
			return err
		}
	}
	return scanner.Err()
}

func replaceLine(text []byte) []byte {
	// There should only be a single match.
	submatches := defPattern.FindSubmatch(text)
	if submatches == nil {
		return replaceVars(text)
	}
	spacing := submatches[1]
	isBinary := len(submatches[2]) == 2
	varName := submatches[3]

	var result bytes.Buffer
	if isBinary {
		result.WriteByte('#')
		result.Write(spacing)
		result.WriteString("define ")
		result.Write(varName)
		result.WriteByte(' ')
		if varMap.Defined(string(varName)) {
			result.WriteByte('1')
		} else {
			result.WriteByte('0')
		}
	} else if varMap.Defined(string(varName)) {
		result.WriteByte('#')
		result.Write(spacing)
		result.WriteString("define ")
		result.Write(varName)
		result.Write(replaceVars(text[len(submatches[0]):]))
	} else {
		result.WriteString("/* #undef ")
		result.Write(varName)
		result.WriteString(" */")
	}
	return result.Bytes()
}

func replaceVars(text []byte) []byte {
	pattern := allPattern
	if *atOnly {
		pattern = atPattern
	}
	return pattern.ReplaceAllFunc(text, func(key []byte) []byte {
		// Chop off the delimiters before looking up the variable.
		if key[0] == '@' {
			key = key[1 : len(key)-1]
		} else {
			key = key[2 : len(key)-1]
		}
		return []byte(varMap.Get(string(key)))
	})
}

func main() {
	flag.Parse()
	if flag.NArg() != 1 {
		fmt.Fprintf(flag.CommandLine.Output(), "Missing required input file\n")
		flag.Usage()
		os.Exit(1)
	}
	if err := json.Unmarshal([]byte(*jsonValues), varMap); err != nil {
		log.Fatalf("Invalid JSON: %s", err)
	}

	input, err := os.Open(flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}
	defer input.Close()

	output := os.Stdout
	if len(*outFile) > 0 && *outFile != "-" {
		if output, err = os.Create(*outFile); err != nil {
			log.Fatal(err)
		}
		defer output.Close()
	}

	if err := transform(input, output); err != nil {
		log.Fatalf("Error processing file: %s", err)
	}
}
