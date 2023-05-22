/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

package mergecmd

import (
	"fmt"
	"io/ioutil"

	"kythe.io/kythe/go/util/vnameutil"
)

// vnameRules is a path-valued flag used for loading VName mapping rules from a vnames.json file.
type vnameRules struct {
	filename string
	vnameutil.Rules
}

// Set implements part of the flag.Value interface and will set a new filename for the flag.
func (f *vnameRules) Set(s string) error {
	f.filename = s
	data, err := ioutil.ReadFile(f.filename)
	if err != nil {
		return fmt.Errorf("reading vname rules: %v", err)
	}
	f.Rules, err = vnameutil.ParseRules(data)
	if err != nil {
		return fmt.Errorf("reading vname rules: %v", err)
	}
	return nil
}

// String implements part of the flag.Value interface and returns a string-ish value for the flag.
func (f *vnameRules) String() string {
	return f.filename
}

// Get implements part of the flag.Getter interface.
func (f *vnameRules) Get() any {
	return f
}
