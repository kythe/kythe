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

// Binary runextractor provides a tool to wrap a Java compilation with Kythe's
// custom extractor logic.
//
// Before running this binary, make sure that any required environment variables
// for the underlying javac wrapper are set.  For example the default Kythe
// javac-wrapper.sh has requirements described at
// kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac-wrapper.sh
package main

import (
	"context"
	"flag"
	"os"

	"kythe.io/kythe/go/extractors/config/runextractor/gradlecmd"
	"kythe.io/kythe/go/extractors/config/runextractor/mavencmd"

	"github.com/google/subcommands"
)

const (
	javaGroup = "java"
)

func init() {
	subcommands.Register(gradlecmd.New(), javaGroup)
	subcommands.Register(mavencmd.New(), javaGroup)
}

func main() {
	flag.Parse()
	ctx := context.Background()

	os.Exit(int(subcommands.Execute(ctx)))
}
