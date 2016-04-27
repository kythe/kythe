/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// Binary scan_leveldb is the cat command for LevelDB.  As well as being able to print each
// key-value pair on its own line (or as a JSON object), scan_leveldb is able to decode common Kythe
// protocol buffers stored in each value (see --proto_value).
package main

import (
	"encoding/base64"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"reflect"
	"strconv"
	"strings"

	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/util/flagutil"

	"github.com/golang/protobuf/proto"

	_ "kythe.io/kythe/proto/serving_proto"
	_ "kythe.io/kythe/proto/storage_proto"
)

var (
	emitJSON   = flag.Bool("json", false, "Emit JSON objects instead of a line per key-value")
	lineFormat = flag.String("format", "@key@\t@value@", "Format of each key value line")
	keyPrefix  = flag.String("prefix", "", "Only scan the key range with the given prefix")

	stringKey   = flag.Bool("string_key", true, "Decode each key as a string rather than raw bytes")
	stringValue = flag.Bool("string_value", false, "Decode each value as a string rather than raw bytes (--proto_value overrides this)")
	protoValue  = flag.String("proto_value", "", `Decode each value as the given protocol buffer type (e.g. "kythe.proto.serving.FileDecorations" or "kythe.proto.serving.PagedEdgeSet")`)
)

func init() {
	flag.Usage = flagutil.SimpleUsage(
		"Scan/print each key-value in the given LevelDB(s)",
		"[--string_key=false] [--string_value|--proto_value type]\n[--json] [--format f] [--prefix p] <leveldb-path>+")
}

func main() {
	flag.Parse()

	if flag.NArg() == 0 {
		flagutil.UsageError("Missing path to LevelDB")
	}

	var protoValueType reflect.Type
	if *protoValue != "" {
		protoValueType = proto.MessageType(*protoValue)
		if protoValueType == nil {
			flagutil.UsageErrorf("could not understand protocol buffer type: %q", *protoValue)
		}
	}

	var en *json.Encoder
	if *emitJSON {
		en = json.NewEncoder(os.Stdout)
	}

	for _, path := range flag.Args() {
		func() {
			db, err := leveldb.Open(path, nil)
			if err != nil {
				log.Fatalf("Error opening %q: %v", path, err)
			}
			defer db.Close()

			it, err := db.ScanPrefix([]byte(*keyPrefix), nil)
			if err != nil {
				log.Fatalf("Error creating iterator for %q: v", path, err)
			}
			defer it.Close()

			for {
				key, val, err := it.Next()
				if err == io.EOF {
					break
				} else if err != nil {
					log.Fatalf("Error during scan of %q: %v", path, err)
				}

				var k, v interface{}

				if protoValueType == nil {
					if *stringKey {
						k = strconv.Quote(string(key))
					} else {
						k = base64.StdEncoding.EncodeToString(key)
					}
					if *stringValue {
						v = strconv.Quote(string(val))
					} else {
						v = base64.StdEncoding.EncodeToString(val)
					}
				} else {
					p := reflect.New(protoValueType.Elem()).Interface().(proto.Message)
					if err := proto.Unmarshal(val, p); err != nil {
						log.Fatalf("Error unmarshaling value to %q: %v", *protoValue, err)
					}

					k, v = string(key), p
				}

				if en == nil {
					fmt.Println(strings.NewReplacer(
						"@key@", fmt.Sprintf("%s", k),
						"@value@", fmt.Sprintf("%s", v),
					).Replace(*lineFormat))
				} else {
					en.Encode(keyValue{k, v})
				}
			}
		}()
	}
}

type keyValue struct {
	Key   interface{} `json:"key"`
	Value interface{} `json:"value"`
}
