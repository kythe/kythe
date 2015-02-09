/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package indexpack

// This file adds a VFS implementation that enables an index pack to be read
// from a ZIP file of its root directory.  It is not possible to write to the
// pack in this format, and the methods that support writing will return
// errors.

import (
	"archive/zip"
	"errors"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"golang.org/x/net/context"
)

// The operations that support writing all return errNotSupported for all calls.
var errNotSupported = errors.New("operation not supported")

type readerAt struct {
	sync.Mutex
	rs io.ReadSeeker
}

func (r *readerAt) ReadAt(buf []byte, pos int64) (int, error) {
	r.Lock()
	defer r.Unlock()

	const fromStart = 0
	if _, err := r.rs.Seek(pos, fromStart); err != nil {
		return 0, err
	}
	return r.rs.Read(buf)
}

// OpenZip returns a read-only *Archive tied to the ZIP file at r, which is
// expected to contain the recursive contents of an indexpack directory and its
// subdirectories.  Operations that write to the pack will return errors.
func OpenZip(ctx context.Context, r io.ReadSeeker, opts ...Option) (*Archive, error) {
	const fromEnd = 2
	size, err := r.Seek(0, fromEnd)
	if err != nil {
		return nil, err
	}

	rc, err := zip.NewReader(&readerAt{rs: r}, size)
	if err != nil {
		return nil, err
	}
	if len(rc.File) == 0 {
		return nil, errors.New("archive has no root directory")
	}
	root := rc.File[0].Name
	if i := strings.Index(root, string(filepath.Separator)); i > 0 {
		root = root[:i]
	}
	opts = append(opts, FS(zipFS{rc}))
	pack, err := Open(ctx, root, opts...)
	if err != nil {
		return nil, err
	}
	return pack, nil
}

type zipFS struct {
	pack *zip.Reader
}

func (z zipFS) find(path string) *zip.File {
	dirPath := path + string(filepath.Separator)
	for _, f := range z.pack.File {
		switch f.Name {
		case path, dirPath:
			return f
		}
	}
	return nil
}

// Stat implements part of indexpack.VFS using the file metadata stored in the
// zip archive.  The path must match one of the archive paths.
func (z zipFS) Stat(_ context.Context, path string) (os.FileInfo, error) {
	f := z.find(path)
	if f == nil {
		return nil, fmt.Errorf("path %q does not exist", path)
	}
	return f.FileInfo(), nil
}

// Open implements part of indexpack.VFS, returning a io.ReadCloser owned by
// the underlying zip archive. It is safe to open multiple files concurrently,
// as documented by the zip package.
func (z zipFS) Open(_ context.Context, path string) (io.ReadCloser, error) {
	f := z.find(path)
	if f == nil {
		return nil, os.ErrNotExist
	}
	return f.Open()
}

// Glob implements part of indexpack.VFS using filepath.Match to compare the
// glob pattern to each archive path.
func (z zipFS) Glob(_ context.Context, glob string) ([]string, error) {
	var names []string
	for _, f := range z.pack.File {
		if ok, err := filepath.Match(glob, f.Name); err != nil {
			log.Panicf("Invalid glob pattern %q: %v", glob, err)
		} else if ok {
			names = append(names, f.Name)
		}
	}
	return names, nil
}

func (zipFS) Create(_ context.Context, _ string) (io.WriteCloser, error) { return nil, errNotSupported }
func (zipFS) MkdirAll(_ context.Context, _ string, _ os.FileMode) error  { return errNotSupported }
func (zipFS) Rename(_ context.Context, _, _ string) error                { return errNotSupported }
func (zipFS) Remove(_ context.Context, _ string) error                   { return errNotSupported }
