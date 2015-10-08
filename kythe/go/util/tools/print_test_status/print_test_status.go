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

// Binary print_test_status takes a single argument that is a path to file with
// an protobuf wire-encoding of a Bazel TestResultData and prints it as JSON to
// stdout.
package main

import (
	"encoding/json"
	"flag"
	"io/ioutil"
	"log"
	"os"

	"kythe.io/kythe/go/platform/vfs"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	tspb "kythe.io/third_party/bazel/test_status_proto"
)

func main() {
	flag.Parse()

	f, err := vfs.Open(context.Background(), flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}

	data, err := ioutil.ReadAll(f)
	f.Close()
	if err != nil {
		log.Fatal(err)
	}

	var tr tspb.TestResultData
	if err := proto.Unmarshal(data, &tr); err != nil {
		log.Fatal(err)
	}

	if err := json.NewEncoder(os.Stdout).Encode(&tr); err != nil {
		log.Fatal(err)
	}

}
