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

// Package zip defines a VFS implementation that understands a zip archive as an
// isolated, read-only file system.
package zip // import "kythe.io/kythe/go/platform/vfs/zip"

import (
	"archive/zip"
	"context"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/platform/vfs"
)

var _ vfs.Reader = FS{}

// Open returns a read-only virtual file system (vfs.Reader), using the contents
// a zip archive read with r.
func Open(r io.ReaderAt, size int64) (FS, error) {
	rc, err := zip.NewReader(r, size)
	if err != nil {
		return FS{}, err
	}
	if len(rc.File) == 0 {
		return FS{}, errors.New("archive has no root directory")
	}
	return FS{rc}, err
}

// FS implements the vfs.Reader interface for zip archives.
type FS struct{ Archive *zip.Reader }

func (z FS) find(path string) *zip.File {
	dirPath := path + string(filepath.Separator)
	for _, f := range z.Archive.File {
		switch f.Name {
		case path, dirPath:
			return f
		}
	}
	return nil
}

// Stat implements part of vfs.Reader using the file metadata stored in the
// zip archive.  The path must match one of the archive paths.
func (z FS) Stat(_ context.Context, path string) (os.FileInfo, error) {
	f := z.find(path)
	if f == nil {
		return nil, fmt.Errorf("path %q does not exist", path)
	}
	return f.FileInfo(), nil
}

// Open implements part of vfs.Reader, returning a vfs.FileReader owned by
// the underlying zip archive. It is safe to open multiple files concurrently,
// as documented by the zip package.
func (z FS) Open(_ context.Context, path string) (vfs.FileReader, error) {
	f := z.find(path)
	if f == nil {
		return nil, os.ErrNotExist
	}
	fo, err := f.Open()
	if err != nil {
		return nil, err
	}
	return vfs.UnseekableFileReader{fo}, nil
}

// Glob implements part of vfs.Reader using filepath.Match to compare the
// glob pattern to each archive path.
func (z FS) Glob(_ context.Context, glob string) ([]string, error) {
	var names []string
	for _, f := range z.Archive.File {
		if ok, err := filepath.Match(glob, f.Name); err != nil {
			panic(fmt.Sprintf("Invalid glob pattern %q: %v", glob, err))
		} else if ok {
			names = append(names, f.Name)
		}
	}
	return names, nil
}
