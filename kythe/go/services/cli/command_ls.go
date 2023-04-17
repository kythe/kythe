/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package cli

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"path/filepath"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/util/kytheuri"

	ftpb "kythe.io/kythe/proto/filetree_go_proto"
)

type lsCommand struct {
	baseKytheCommand
	lsURIs         bool
	filesOnly      bool
	dirsOnly       bool
	includeMissing bool
}

func (lsCommand) Name() string     { return "ls" }
func (lsCommand) Synopsis() string { return "list a directory's contents" }
func (c *lsCommand) SetFlags(flag *flag.FlagSet) {
	flag.BoolVar(&c.lsURIs, "uris", false, "Display files/directories as Kythe URIs")
	flag.BoolVar(&c.filesOnly, "files", false, "Display only files")
	flag.BoolVar(&c.dirsOnly, "dirs", false, "Display only directories")
	flag.BoolVar(&c.includeMissing, "include_files_missing_text", false, "Include files missing text")
}
func (c lsCommand) Run(ctx context.Context, flag *flag.FlagSet, api API) error {
	if c.filesOnly && c.dirsOnly {
		return errors.New("--files and --dirs are mutually exclusive")
	}

	if len(flag.Args()) == 0 {
		req := &ftpb.CorpusRootsRequest{}
		LogRequest(req)
		cr, err := api.FileTreeService.CorpusRoots(ctx, req)
		if err != nil {
			return err
		}
		return c.displayCorpusRoots(cr)
	}
	var corpus, root, path string
	switch len(flag.Args()) {
	case 1:
		uri, err := kytheuri.Parse(flag.Arg(0))
		if err != nil {
			return fmt.Errorf("invalid uri %q: %v", flag.Arg(0), err)
		}
		corpus = uri.Corpus
		root = uri.Root
		path = uri.Path
	default:
		return fmt.Errorf("too many arguments given: %v", flag.Args())
	}
	path = filetree.CleanDirPath(path)
	req := &ftpb.DirectoryRequest{
		Corpus: corpus,
		Root:   root,
		Path:   path,

		IncludeFilesMissingText: c.includeMissing,
	}
	LogRequest(req)
	dir, err := api.FileTreeService.Directory(ctx, req)
	if err != nil {
		return err
	}

	if c.filesOnly {
		dir.Entry = filterEntries(dir.Entry, ftpb.DirectoryReply_FILE)
	} else if c.dirsOnly {
		dir.Entry = filterEntries(dir.Entry, ftpb.DirectoryReply_DIRECTORY)
	}

	return c.displayDirectory(dir)
}

func filterEntries(entries []*ftpb.DirectoryReply_Entry, kind ftpb.DirectoryReply_Kind) []*ftpb.DirectoryReply_Entry {
	var j int
	for _, e := range entries {
		if e.Kind == kind {
			entries[j] = e
			j++
		}
	}
	return entries[:j]
}

func (c lsCommand) displayCorpusRoots(cr *ftpb.CorpusRootsReply) error {
	if DisplayJSON {
		return PrintJSONMessage(cr)
	}

	for _, corpus := range cr.Corpus {
		for _, root := range corpus.Root {
			var err error
			if c.lsURIs {
				uri := kytheuri.URI{
					Corpus: corpus.Name,
					Root:   root,
				}
				_, err = fmt.Fprintln(out, uri.String())
			} else {
				_, err = fmt.Fprintln(out, filepath.Join(corpus.Name, root))
			}
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func (c lsCommand) displayDirectory(d *ftpb.DirectoryReply) error {
	if DisplayJSON {
		return PrintJSONMessage(d)
	}

	for _, e := range d.Entry {
		name := e.Name
		if c.lsURIs {
			uri := &kytheuri.URI{
				Corpus: d.Corpus,
				Root:   d.Root,
				Path:   filepath.Join(d.Path, name),
			}
			name = uri.String()
		} else if e.Kind == ftpb.DirectoryReply_DIRECTORY {
			name += "/"
		}

		if _, err := fmt.Fprintln(out, name); err != nil {
			return err
		}
	}
	return nil
}
