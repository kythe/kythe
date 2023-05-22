/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Binary print_extra_action takes a single argument that is a path to file with
// an protobuf wire-encoding of a Bazel ExtraActionInfo and prints it as JSON to
// stdout.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"io/ioutil"
	"os"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/log"

	"github.com/golang/protobuf/proto"

	xapb "kythe.io/third_party/bazel/extra_actions_base_go_proto"
)

var knownExtensions = []*proto.ExtensionDesc{
	xapb.E_SpawnInfo_SpawnInfo,
	xapb.E_CppCompileInfo_CppCompileInfo,
	xapb.E_CppLinkInfo_CppLinkInfo,
	xapb.E_JavaCompileInfo_JavaCompileInfo,
	xapb.E_PythonInfo_PythonInfo,
}

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

	var xa xapb.ExtraActionInfo
	if err := proto.Unmarshal(data, &xa); err != nil {
		log.Fatal(err)
	}

	obj := make(map[string]any)
	obj["extra_action_info"] = &xa

	xs, err := proto.GetExtensions(&xa, knownExtensions)
	if err != nil {
		log.Fatal(err)
	}

	var extensions []any
	for _, e := range xs {
		if e != nil {
			extensions = append(extensions, e)
		}
	}

	if len(extensions) > 0 {
		obj["extensions"] = extensions
	}

	if err := json.NewEncoder(os.Stdout).Encode(obj); err != nil {
		log.Fatal(err)
	}

}
