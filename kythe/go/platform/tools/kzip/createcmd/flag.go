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

package createcmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"sort"
	"strings"

	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/vnameutil"

	"google.golang.org/protobuf/encoding/protojson"

	anypb "github.com/golang/protobuf/ptypes/any"
	apb "kythe.io/kythe/proto/analysis_go_proto"
)

type repeatedString []string

// Set implements part of the flag.Getter interface and will append a new value to the flag.
func (f *repeatedString) Set(s string) error {
	*f = append(*f, s)
	return nil
}

// String implements part of the flag.Getter interface and returns a string-ish value for the flag.
func (f *repeatedString) String() string {
	if f == nil {
		return ""
	}
	return strings.Join(*f, ",")
}

// Get implements flag.Getter and returns a slice of string values.
func (f *repeatedString) Get() any {
	if f == nil {
		return []string(nil)
	}
	return *f
}

type repeatedEnv map[string]string

// Set implements part of the flag.Getter interface and will append a new value to the flag.
func (f *repeatedEnv) Set(s string) error {
	parts := strings.SplitN(s, "=", 2)
	if len(parts) != 2 {
		return fmt.Errorf("invalid environment entry: %v", s)
	}
	if *f == nil {
		*f = make(map[string]string)
	}
	(*f)[parts[0]] = parts[1]
	return nil
}

// String implements part of the flag.Getter interface and returns a string-ish value for the flag.
func (f *repeatedEnv) String() string {
	if f == nil || *f == nil {
		return ""
	}
	var values []string
	for key, value := range *f {
		values = append(values, key+"="+value)
	}
	return strings.Join(values, "\n")
}

// Get implements flag.Getter and returns a slice of string values.
func (f *repeatedEnv) Get() any {
	if f == nil {
		return map[string]string(nil)
	}
	return *f
}

// ToProto returns a []*apb.CompilationUnit_Env for the mapped environment.
func (f *repeatedEnv) ToProto() []*apb.CompilationUnit_Env {
	if f == nil || *f == nil {
		return nil
	}
	var result []*apb.CompilationUnit_Env
	for key, value := range *f {
		result = append(result, &apb.CompilationUnit_Env{
			Name:  key,
			Value: value,
		})
	}
	sort.Slice(result, func(i, j int) bool { return result[i].Name < result[j].Name })
	return result
}

type kytheURI kytheuri.URI

// Set implements part of the flag.Value interface and will append a new value to the flag.
func (f *kytheURI) Set(s string) error {
	uri, err := kytheuri.Parse(s)
	switch {
	case err != nil:
		return err
	case uri.Corpus == "":
		return errors.New("missing required URI field: corpus")
	case uri.Language == "":
		return errors.New("missing required URI field: language")

	}
	*f = *(*kytheURI)(uri)
	return err
}

// String implements part of the flag.Value interface and returns a string-ish value for the flag.
func (f *kytheURI) String() string {
	return (*kytheuri.URI)(f).String()
}

// Get implements part of the flag.Getter interface.
func (f *kytheURI) Get() any {
	return f
}

// repeatedAny is a repeated flag of JSON-encoded Any messages.
type repeatedAny []*anypb.Any

// Set implements part of the flag.Getter interface and will append a new value to the flag.
func (f *repeatedAny) Set(s string) error {
	dec := json.NewDecoder(strings.NewReader(s))
	for dec.More() {
		var raw json.RawMessage
		if err := dec.Decode(&raw); err != nil {
			return err
		}
		var detail anypb.Any
		if err := protojson.Unmarshal(raw, &detail); err != nil {
			return err
		}
		*f = append(*f, &detail)
	}
	return nil
}

var toJSON = &protojson.MarshalOptions{UseProtoNames: true}

// String implements part of the flag.Getter interface and returns a string-ish value for the flag.
func (f *repeatedAny) String() string {
	if f == nil {
		return ""
	}
	var result []string
	for _, val := range *f {
		rec, err := toJSON.Marshal(val)
		if err != nil {
			panic(err)
		}
		result = append(result, string(rec))
	}
	return strings.Join(result, " ")
}

// Get implements flag.Getter and returns a slice of string values.
func (f *repeatedAny) Get() any {
	if f == nil {
		return []*anypb.Any(nil)
	}
	return *f
}

// vnameRules is a path-valued flag used for loading VName mapping rules from a vnames.json file.
type vnameRules struct {
	filename string
	vnameutil.Rules
}

// Set implements part of the flag.Value interface and will append a new value to the flag.
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
