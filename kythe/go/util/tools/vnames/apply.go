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

package main

import (
	"bufio"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/vnameutil"

	"github.com/google/subcommands"
	"google.golang.org/protobuf/encoding/protojson"
)

type rulesFormat string

const (
	jsonFormat  rulesFormat = "JSON"
	protoFormat rulesFormat = "PROTO"
)

type applyRulesCmd struct {
	cmdutil.Info

	rulesPath string
	format    string

	allowEmpty bool
}

var applyRulesInfo = cmdutil.NewInfo("apply-rules", "apply VName rewrite rules to stdin lines",
	`Usage: apply-rules --rules <path> [--format proto]`)

func (c *applyRulesCmd) SetFlags(flag *flag.FlagSet) {
	flag.StringVar(&c.rulesPath, "rules", "", "Path to VName rewrite rules file")
	flag.StringVar(&c.format, "format", string(jsonFormat), `Path of VName rewrite rules file {"JSON", "PROTO"}`)
	flag.BoolVar(&c.allowEmpty, "allow_empty", false, "Whether to write an empty VName when no rule matches the input")
}
func (c *applyRulesCmd) Execute(ctx context.Context, flag *flag.FlagSet, args ...any) subcommands.ExitStatus {
	rules, err := rulesFormat(strings.ToUpper(c.format)).readFile(ctx, c.rulesPath)
	if err != nil {
		return cmdErrorf("reading %q: %v", c.rulesPath, err)
	}
	s := bufio.NewScanner(os.Stdin)
	m := protojson.MarshalOptions{UseProtoNames: true, Indent: "  "}
	for s.Scan() {
		line := s.Text()
		v, ok := rules.Apply(line)
		if ok || c.allowEmpty {
			rec, err := m.Marshal(v)
			if err != nil {
				return cmdErrorf("encoding VName: %v", err)
			} else if _, err := os.Stdout.Write(rec); err != nil {
				return cmdErrorf("writing VName: %v", err)
			} else if _, err := fmt.Fprintln(os.Stdout, ""); err != nil {
				return cmdErrorf("writing newline: %v", err)
			}
		}
	}
	if err := s.Err(); err != nil {
		return cmdErrorf("reading paths from stdin: %v", err)
	}
	return subcommands.ExitSuccess
}

func (f rulesFormat) readFile(ctx context.Context, path string) (vnameutil.Rules, error) {
	file, err := vfs.Open(ctx, path)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	return f.readRules(file)
}

func (f rulesFormat) readRules(r io.Reader) (vnameutil.Rules, error) {
	switch f {
	case jsonFormat:
		return vnameutil.ReadRules(r)
	case protoFormat:
		rec, err := ioutil.ReadAll(r)
		if err != nil {
			return nil, err
		}
		return vnameutil.ParseProtoRules(rec)
	default:
		return nil, fmt.Errorf("unknown rules format: %q", f)
	}
}

func (f rulesFormat) writeRules(rules vnameutil.Rules, w io.Writer) error {
	switch f {
	case jsonFormat:
		en := json.NewEncoder(w)
		en.SetIndent("", "  ")
		return en.Encode(rules)
	case protoFormat:
		rec, err := rules.Marshal()
		if err != nil {
			return err
		}
		_, err = w.Write(rec)
		return err
	default:
		return fmt.Errorf("unknown rules format: %q", f)
	}
}

func cmdErrorf(msg string, args ...any) subcommands.ExitStatus {
	log.Errorf(msg, args...)
	return subcommands.ExitFailure
}
