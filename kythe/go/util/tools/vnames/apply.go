package main

import (
	"bufio"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"strings"

	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/vnameutil"

	"github.com/golang/protobuf/jsonpb"
	"github.com/google/subcommands"
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
func (c *applyRulesCmd) Execute(ctx context.Context, flag *flag.FlagSet, args ...interface{}) subcommands.ExitStatus {
	rules, err := rulesFormat(strings.ToUpper(c.format)).readFile(ctx, c.rulesPath)
	if err != nil {
		return cmdErrorf("reading %q: %v", c.rulesPath, err)
	}
	s := bufio.NewScanner(os.Stdin)
	m := jsonpb.Marshaler{OrigName: true}
	for s.Scan() {
		line := s.Text()
		v, ok := rules.Apply(line)
		if ok || c.allowEmpty {
			if err := m.Marshal(os.Stdout, v); err != nil {
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
		return json.NewEncoder(w).Encode(rules)
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

func cmdErrorf(msg string, args ...interface{}) subcommands.ExitStatus {
	log.Printf("ERROR: "+msg, args...)
	return subcommands.ExitFailure
}
