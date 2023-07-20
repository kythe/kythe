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

// Package flagutil is a collection of helper functions for Kythe binaries using
// the flag package.
package flagutil // import "kythe.io/kythe/go/util/flagutil"

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"

	"kythe.io/kythe/go/util/build"

	"bitbucket.org/creachadair/stringset"
)

// SimpleUsage returns a basic flag.Usage function that prints the given
// description and list of arguments in the following format:
//
//	Usage: binary <arg0> <arg1> ... <argN>
//	<description>
//
//	<build.VersionLine()>
//
//	Flags:
//	<flag.PrintDefaults()>
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
func UsageErrorf(str string, vals ...any) {
	UsageError(fmt.Sprintf(str, vals...))
}

// StringList implements a flag.Value that accepts an sequence of values as a CSV.
type StringList []string

// Set implements part of the flag.Getter interface and will append new values to the flag.
func (f *StringList) Set(s string) error {
	*f = append(*f, strings.Split(s, ",")...)
	return nil
}

// String implements part of the flag.Getter interface and returns a string-ish value for the flag.
func (f *StringList) String() string {
	if f == nil {
		return ""
	}
	return strings.Join(*f, ",")
}

// Get implements flag.Getter and returns a slice of string values.
func (f *StringList) Get() any {
	if f == nil {
		return []string(nil)
	}
	return *f
}

// StringSet implements a flag.Value that accepts an set of values as a CSV.
type StringSet stringset.Set

// Set implements part of the flag.Getter interface and will append new values to the flag.
func (f *StringSet) Set(s string) error {
	(*stringset.Set)(f).Add(strings.Split(s, ",")...)
	return nil
}

// Update adds the values from other to the contained stringset.
func (f *StringSet) Update(o StringSet) bool {
	return (*stringset.Set)(f).Update(stringset.Set(o))
}

// Elements returns the set of elements as a sorted slice.
func (f *StringSet) Elements() []string {
	return (*stringset.Set)(f).Elements()
}

// Len returns the number of elements.
func (f *StringSet) Len() int {
	return (*stringset.Set)(f).Len()
}

// String implements part of the flag.Getter interface and returns a string-ish value for the flag.
func (f *StringSet) String() string {
	if f == nil {
		return ""
	}
	return strings.Join(f.Elements(), ",")
}

// Get implements flag.Getter and returns a slice of string values.
func (f *StringSet) Get() any {
	if f == nil {
		return stringset.Set(nil)
	}
	return *f
}

// StringMultimap implements a flag.Value that accepts an set of key-value entries as a CSV.
type StringMultimap map[string]stringset.Set

// Set implements part of the flag.Getter interface and will append new values to the flag.
func (f *StringMultimap) Set(s string) error {
	if *f == nil {
		*f = make(map[string]stringset.Set)
	}
	m := *f
	for _, e := range strings.Split(s, ",") {
		pair := strings.SplitN(e, "=", 2)
		if len(pair) != 2 {
			return fmt.Errorf("invalid key-value entry: %q", e)
		}
		key, val := pair[0], pair[1]
		set := m[key]
		if set == nil {
			set = stringset.New()
			m[key] = set
		}
		set.Add(val)
	}
	return nil
}

// String implements part of the flag.Getter interface and returns a string-ish value for the flag.
func (f *StringMultimap) String() string {
	if f == nil || *f == nil {
		return "{}"
	}
	entries := make([]string, 0, len(*f))
	for k, vs := range *f {
		for _, v := range vs.Elements() {
			entries = append(entries, fmt.Sprintf("%s=%s", k, v))
		}
	}
	sort.Strings(entries)
	return strings.Join(entries, ",")
}

// Get implements flag.Getter and returns a slice of string values.
func (f *StringMultimap) Get() any {
	if f == nil {
		return map[string]stringset.Set(nil)
	}
	return *f
}

// IntList implements a flag.Value that accepts multiple values by repeatedly specifying the flag.
type IntList []int

// String returns a string representation of the flag's value.
func (i *IntList) String() string { return fmt.Sprintf("%v", *i) }

// Set adds a value to the flag's list of integers.
func (i *IntList) Set(value string) error {
	v, err := strconv.Atoi(value)
	if err != nil {
		return err
	}
	*i = append(*i, v)
	return nil
}
