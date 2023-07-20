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

// Package viewcmd displays the contents of compilation units stored in .kzip files.
package viewcmd // import "kythe.io/kythe/go/platform/tools/kzip/viewcmd"

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"time"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/log"

	"github.com/google/subcommands"
	"golang.org/x/sync/errgroup"
	"golang.org/x/sync/semaphore"
	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"
)

type cmd struct {
	cmdutil.Info
	extractDir string
}

// New returns an implementation of the "view" subcommand.
func New() subcommands.Command {
	const usage = `Usage: view [options] <file-path>...

Print or extract compilation units stored in .kzip files.  By default, the
compilation unit is printed to stdout in JSON format.

With -extract the compilation record and the full contents of all the required
input files are extracted into the named directory, preserving the path
structure specified in the compilation record.`

	return &cmd{
		Info: cmdutil.NewInfo("view", "view compilation units stored in .kzip files", usage),
	}
}

// SetFlags implements part of subcommands.Command.
func (c *cmd) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&c.extractDir, "extract", "", "Extract files to this directory")
}

// Execute implements part of subcommands.Command.
func (c *cmd) Execute(ctx context.Context, fs *flag.FlagSet, args ...any) subcommands.ExitStatus {
	var hasErrors bool
	for _, path := range fs.Args() {
		ext := filepath.Ext(path)
		base := filepath.Base(strings.TrimSuffix(path, ext))

		f, err := vfs.Open(ctx, path)
		if err != nil {
			log.Errorf("opening .kzip file: %v", err)
			hasErrors = true
			continue
		}
		defer f.Close()

		err = kzip.Scan(f, func(r *kzip.Reader, unit *kzip.Unit) error {
			if err := c.writeUnit(base+"-"+unit.Digest, unit.Proto); err != nil {
				return fmt.Errorf("writing unit: %v", err)
			} else if fd, err := c.kzipFiles(r, unit); err != nil {
				return fmt.Errorf("extracting files: %v", err)
			} else {
				return c.writeFiles(fd)
			}
		})
		if err != nil {
			log.Errorf("scanning .kzip file: %v", err)
			hasErrors = true
		}
	}
	if hasErrors {
		return subcommands.ExitFailure
	}
	return subcommands.ExitSuccess
}

var marshaler = &protojson.MarshalOptions{UseProtoNames: true}

func (c *cmd) writeUnit(base string, msg proto.Message) error {
	if c.extractDir == "" {
		rec, err := marshaler.Marshal(msg)
		if err != nil {
			return err
		}
		_, err = os.Stdout.Write(rec)
		if err == nil {
			fmt.Println()
		}
		return err
	}
	if err := os.MkdirAll(c.extractDir, 0750); err != nil {
		return fmt.Errorf("creating output directory: %v", err)
	}
	f, err := os.Create(filepath.Join(c.extractDir, base+".unit"))
	if err != nil {
		return err
	}
	rec, err := marshaler.Marshal(msg)
	if err != nil {
		f.Close()
		return err
	}
	_, err = f.Write(rec)
	cerr := f.Close()
	if err != nil {
		return err
	}
	return cerr
}

func (c *cmd) writeFiles(fd []fileData) error {
	if c.extractDir == "" {
		return nil
	}
	g, ctx := errgroup.WithContext(context.Background())
	start := time.Now()
	defer func() { log.Infof("Extracted %d files in %v", len(fd), time.Since(start)) }()
	sem := semaphore.NewWeighted(64) // limit concurrency on large compilations
	for _, file := range fd {
		file := file
		g.Go(func() error {
			sem.Acquire(ctx, 1)
			defer sem.Release(1)

			path := filepath.Join(c.extractDir, file.path)
			dir := filepath.Dir(path)
			if err := os.MkdirAll(dir, 0750); err != nil {
				return err
			}
			return ioutil.WriteFile(path, file.data, 0644)
		})
	}
	return g.Wait()
}

type fileData struct {
	path string
	data []byte
}

// kzipFiles extracts the file paths and contents from a kzip compilation.
func (c *cmd) kzipFiles(r *kzip.Reader, unit *kzip.Unit) ([]fileData, error) {
	out := make([]fileData, len(unit.Proto.RequiredInput))
	for i, ri := range unit.Proto.RequiredInput {
		info := ri.GetInfo()
		out[i].path = info.GetPath()
		data, err := r.ReadAll(info.GetDigest())
		if err != nil {
			return nil, err
		}
		out[i].data = data
	}
	return out, nil
}
