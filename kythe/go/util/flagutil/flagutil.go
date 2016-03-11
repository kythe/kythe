/*
 * Copyright 2015 Google Inc. All rights reserved.
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

// Package flagutil is a collection of helper functions for Kythe binaries using
// the flag package.
package flagutil

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/util/build"
)

// SimpleUsage returns a basic flag.Usage function that prints the given
// description and list of arguments in the following format:
//
//  Usage: binary <arg0> <arg1> ... <argN>
//  <description>
//
//  <build.VersionLine()>
//
//  Flags:
//  <flag.PrintDefaults()>
func SimpleUsage(description string, args ...string) func() {
	return func() {
		prefix := fmt.Sprintf("Usage: %s ", filepath.Base(os.Args[0]))
		alignArgs(len(prefix), args)
		fmt.Fprintf(os.Stderr, `%s%s
%s

%s

Flags:
`, prefix, strings.Join(args, " "), description, build.VersionLine())
		flag.PrintDefaults()
	}
}

func alignArgs(col int, args []string) {
	s := strings.Repeat(" ", col)
	for i, arg := range args {
		args[i] = strings.Replace(arg, "\n", "\n"+s, -1)
	}
}

// UsageError prints msg to stderr, calls flag.Usage, and exits the program
// unsuccessfully.
func UsageError(msg string) {
	fmt.Fprintln(os.Stderr, "ERROR: "+msg)
	flag.Usage()
	os.Exit(1)
}

// UsageErrorf prints str formatted with the given vals to stderr, calls
// flag.Usage, and exits the program unsuccessfully.
func UsageErrorf(str string, vals ...interface{}) {
	UsageError(fmt.Sprintf(str, vals...))
}
