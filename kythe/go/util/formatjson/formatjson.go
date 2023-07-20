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

// Binary formatjson reads json from stdin and writes formatted json to stdout.
package main

import (
	"encoding/json"
	"os"

	"kythe.io/kythe/go/util/log"
)

func main() {
	obj := make(map[string]any)
	d := json.NewDecoder(os.Stdin)
	if err := d.Decode(&obj); err != nil {
		log.Fatal(err)
	}

	e := json.NewEncoder(os.Stdout)
	e.SetIndent("", "    ")
	if err := e.Encode(obj); err != nil {
		log.Fatal(err)
	}
}
