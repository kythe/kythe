/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Package archive provides support for reading the contents of archives such
// as .zip and .tar files.
package archive // import "kythe.io/kythe/go/util/archive"

import (
	"archive/tar"
	"archive/zip"
	"compress/bzip2"
	"compress/gzip"
	"errors"
	"fmt"
	"io"
	"path/filepath"
	"strings"
)

// File defines the input capabilities needed to scan an archive file.
type File interface {
	io.Closer
	io.Reader
	io.ReaderAt
	io.Seeker
}

// ErrNotArchive is returned by Scan when passed a file it does not recognize
// as a readable archive.
var ErrNotArchive = errors.New("not a supported archive file")

// A ScanFunc is invoked by the Scan function for each file found in the
// specified archive. The arguments are the filename as encoded in the archive,
// and either an error or a reader positioned at the beginning of the file's
// contents.
//
// Any error returned by the ScanFunc is propagated to the caller of Scan,
// terminating the traversal of the archive.  The callback may choose to ignore
// err, in which case the error is ignored and scanning continues.
type ScanFunc func(filename string, err error, r io.Reader) error

// Scan sequentially scans the contents of an archive and invokes f for each
// file found. If f returns an error, scanning stops and that error is returned
// to the caller of Scan. The path is used to determine what type of archive is
// referred to by file. If the type is not known, it returns ErrNotArchive.
//
// The supported archive formats are:
//
//    .zip     -- ZIP archive (also .ZIP, .jar)
//    .tar     -- uncompressed tar
//    .tar.gz  -- gzip-compressed tar (also .tgz)
//    .tar.bz2 -- bzip2-compressed tar
//
// Scan only invokes f for file entries; directories are not included.
func Scan(file File, path string, f ScanFunc) error {
	format, compression := parsePath(path)
	switch format {
	case ".zip":
		size, err := file.Seek(0, io.SeekEnd)
		if err != nil {
			return fmt.Errorf("archive: finding ZIP file size: %v", err)
		}
		archive, err := zip.NewReader(file, size)
		if err != nil {
			return fmt.Errorf("archive: opening ZIP reader: %v", err)
		}

		for _, entry := range archive.File {
			rc, err := entry.Open()
			err = f(entry.Name, err, rc)
			rc.Close()
			if err != nil {
				return err
			}
		}

	case ".tar":
		r := io.Reader(file)
		switch compression {
		case ".gz":
			gz, err := gzip.NewReader(file)
			if err != nil {
				return fmt.Errorf("archive: opening gzip reader: %v", err)
			}
			r = gz
		case ".bz2":
			r = bzip2.NewReader(file)
		case "":
		default:
		}
		archive := tar.NewReader(r)

		for {
			entry, err := archive.Next()
			if err == io.EOF {
				break
			}
			isFile := entry != nil && entry.FileInfo().Mode().IsRegular()

			// If we got an entry of any kind, invoke the callback whether or
			// not we have an error. If we didn't get an entry, treat an error
			// here as fatal.
			if err == nil {
				err = f(entry.Name, nil, archive)
			} else if isFile {
				err = f(entry.Name, err, nil)
			}
			if err != nil {
				return err
			}
		}

	default:
		return ErrNotArchive
	}
	return nil
}

// parsePath determines which file format is represented by path, returning the
// base file format (.zip or .tar) and the additional compression format
// extension (.gz or .bz2), or "" if there is no additional compression.
// Returns "", "" if the format could not be determined.
func parsePath(path string) (format, compression string) {
	switch ext := filepath.Ext(path); ext {
	case ".zip", ".ZIP", ".jar":
		return ".zip", ""
	case ".tar":
		return ext, ""
	case ".tgz":
		return ".tar", ".gz"
	case ".gz", ".bz2":
		base := filepath.Ext(strings.TrimSuffix(path, ext))
		if base == ".tar" {
			return base, ext
		}
	}
	return "", "" // format unknown
}
