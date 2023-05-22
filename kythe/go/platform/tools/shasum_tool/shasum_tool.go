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

// Binary shasum_tool calcuates the sha256 sum of the specified file.
//
// This is useful in environments which don't have analogous GNU tools
// installed by default.
// Example:
//
//	shasum_tool <input_file>
package main

import (
	"crypto/sha256"
	"encoding/hex"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/util/log"
)

func main() {
	flag.Parse()
	if flag.NArg() != 1 {
		log.Fatal("Usage: shasum_tool <input_file>")
	}

	inputFile := flag.Arg(0)
	f, err := os.Open(inputFile)
	if err != nil {
		log.Fatal("Open: ", err)
	}
	defer f.Close()

	hasher := sha256.New()
	if _, err := io.Copy(hasher, f); err != nil {
		log.Fatal("Hashing failed: ", err)
	}
	fmt.Printf("%s\t%s\n", hex.EncodeToString(hasher.Sum(nil)), filepath.Base(inputFile))
}
