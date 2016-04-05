/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package indexpack provides an interface to a collection of compilation units
// stored in an "index pack" directory structure.  The index pack format is
// defined in kythe-index-pack.txt.
//
// Example usage, writing:
//   pack, err := indexpack.Create(ctx, "path/to/some/directory")
//   if err != nil {
//     log.Exit(err)
//   }
//   for _, cu := range fetchUnits() {
//     if _, err := pack.WriteUnit(ctx, "kythe", cu); err != nil {
//       log.Error(err)
//       continue
//     }
//     for _, input := range cu.RequiredInput {
//       data, err := readFile(input)
//       if err != nil {
//         log.Error(err)
//         continue
//       }
//       if err := pack.WriteFile(ctx, data); err != nil {
//         log.Error(err)
//       }
//     }
//   }
//
// Example usage, reading:
//   pack, err := indexpack.Open(ctx, "some/dir/path", indexpack.UnitType((*cpb.CompilationUnit)(nil)))
//   if err != nil {
//     log.Exit(err)
//   }
//
//   // The value passed to the callback will have the concrete type of
//   // the third parameter to Open (or Create).
//   err := pack.ReadUnits(ctx, "kythe", func(digest string, cu interface{}) error {
//     for _, input := range cu.(*cpb.CompilationUnit).RequiredInput {
//       bits, err := pack.ReadFile(ctx, input.Digest)
//       if err != nil {
//         return err
//       }
//       processData(bits)
//     }
//     return doSomethingUseful(cu)
//   })
//   if err != nil {
//     log.Exit(err)
//   }
package indexpack

import (
	"bufio"
	"compress/gzip"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"strings"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/platform/vfs/zip"

	"github.com/pborman/uuid"
	"golang.org/x/net/context"
)

const (
	dataDir    = "files"
	unitDir    = "units"
	dataSuffix = ".data" // Filename suffix for a file-data file
	unitSuffix = ".unit" // Filename suffix for a compilation unit file
	newSuffix  = ".new"  // Filename suffix for a temporary file used during writing
)

// A unitWrapper captures the top-level JSON structure for a compilation unit
// stored in an index pack.
type unitWrapper struct {
	Format  string          `json:"format"`
	Content json.RawMessage `json:"content"`
}

type packFetcher struct {
	context.Context
	*Archive
}

// Fetch implements analysis.Fetcher by fetching the digest from the index
// pack.  This implementation ignores the path, since there is no direct
// mapping for paths in an index pack.
func (p packFetcher) Fetch(path, digest string) ([]byte, error) {
	return p.Archive.ReadFile(p.Context, digest)
}

// Fetcher returns an analysis.Fetcher that reads file contents from a.
func (a *Archive) Fetcher(ctx context.Context) analysis.Fetcher { return packFetcher{ctx, a} }

// Archive represents an index pack directory.
type Archive struct {
	root     string        // The root path of the index pack
	unitType reflect.Type  // The concrete value type for the ReadUnits callback
	fs       vfs.Interface // Filesystem implementation used for file access
}

// An Option is a configurable setting for an Archive.
type Option func(*Archive) error

// UnitType returns an Option that sets the concrete type used to unmarshal
// compilation units in the ReadUnits method.  If t != nil, the interface value
// passed to the callback will be a pointer to the type of t.
func UnitType(t interface{}) Option {
	return func(a *Archive) error {
		a.unitType = reflect.TypeOf(t)
		if a.unitType != nil && a.unitType.Kind() == reflect.Ptr {
			a.unitType = a.unitType.Elem()
		}
		return nil
	}
}

// FS returns an Option that sets the filesystem interface used to implement
// the index pack.
func FS(fs vfs.Interface) Option {
	return func(a *Archive) error {
		if fs == nil {
			return errors.New("invalid vfs.Interface")
		}
		a.fs = fs
		return nil
	}
}

// FSReader returns an Option that sets the filesystem interface used to
// implement the index pack.  The resulting index pack will be read-only;
// methods such as WriteUnit and WriteFile will return with an error.
func FSReader(fs vfs.Reader) Option { return FS(vfs.UnsupportedWriter{fs}) }

// Create creates a new empty index pack at the specified path. It is an error
// if the path already exists.
func Create(ctx context.Context, path string, opts ...Option) (*Archive, error) {
	a := &Archive{root: path, fs: vfs.Default}
	UnitType((json.RawMessage)(nil))(a) // set default unit type; overridden below
	for _, opt := range opts {
		if err := opt(a); err != nil {
			return nil, err
		}
	}

	if _, err := a.fs.Stat(ctx, path); err == nil {
		return nil, fmt.Errorf("path %q already exists", path)
	}
	if err := a.fs.MkdirAll(ctx, path, 0755); err != nil {
		return nil, err
	}
	if err := a.fs.MkdirAll(ctx, filepath.Join(path, unitDir), 0755); err != nil {
		return nil, err
	}
	if err := a.fs.MkdirAll(ctx, filepath.Join(path, dataDir), 0755); err != nil {
		return nil, err
	}
	return a, nil
}

// Open returns a handle for an existing valid index pack at the specified
// path.  It is an error if path does not exist, or does not have the correct
// format.
func Open(ctx context.Context, path string, opts ...Option) (*Archive, error) {
	a := &Archive{root: path, fs: vfs.Default}
	UnitType((json.RawMessage)(nil))(a) // set default unit type; overridden below
	for _, opt := range opts {
		if err := opt(a); err != nil {
			return nil, err
		}
	}

	fi, err := a.fs.Stat(ctx, path)
	if err != nil {
		return nil, err
	}
	if !fi.IsDir() {
		return nil, fmt.Errorf("path %q is not a directory", path)
	}
	if fi, err := a.fs.Stat(ctx, filepath.Join(path, unitDir)); err != nil || !fi.IsDir() {
		return nil, fmt.Errorf("path %q is missing a units subdirectory", path)
	}
	if fi, err := a.fs.Stat(ctx, filepath.Join(path, dataDir)); err != nil || !fi.IsDir() {
		return nil, fmt.Errorf("path %q is missing a files subdirectory", path)
	}
	return a, nil
}

// OpenZip returns a read-only *Archive tied to the ZIP file at r, whose size
// in bytes is given. The ZIP file is expected to contain the recursive
// contents of an indexpack directory and its subdirectories.  Operations that
// write to the pack will return errors.
func OpenZip(ctx context.Context, r io.ReaderAt, size int64, opts ...Option) (*Archive, error) {
	fs, err := zip.Open(r, size)
	if err != nil {
		return nil, err
	}
	root := fs.Archive.File[0].Name
	if i := strings.Index(root, string(filepath.Separator)); i > 0 {
		root = root[:i]
	}
	opts = append(opts, FSReader(fs))
	pack, err := Open(ctx, root, opts...)
	if err != nil {
		return nil, err
	}
	return pack, nil
}

func (a *Archive) readFile(ctx context.Context, dir, name string) ([]byte, error) {
	f, err := a.fs.Open(ctx, filepath.Join(dir, name))
	if err != nil {
		return nil, err
	}
	defer f.Close()

	gz, err := gzip.NewReader(bufio.NewReader(f))
	if err != nil {
		return nil, err
	}

	return ioutil.ReadAll(gz)
}

// CreateOrOpen opens an existing index pack with the given parameters, if one
// exists; or if not, then attempts to create one.
func CreateOrOpen(ctx context.Context, path string, opts ...Option) (*Archive, error) {
	if a, err := Open(ctx, path, opts...); err == nil {
		return a, nil
	}
	return Create(ctx, path, opts...)
}

func (a *Archive) writeFile(ctx context.Context, dir, name string, data []byte) error {
	tmp := filepath.Join(dir, uuid.New()) + newSuffix
	f, err := a.fs.Create(ctx, tmp)
	if err != nil {
		return err
	}
	// When this function is called, the temp file is garbage; we make a good
	// faith effort to close it and clean it up, but we don't care if it fails.
	cleanup := func() error {
		err := f.Close()
		a.fs.Remove(ctx, tmp)
		return err
	}
	gz := gzip.NewWriter(f)
	if _, err := gz.Write(data); err != nil {
		return cleanup()
	}
	if err := gz.Flush(); err != nil {
		return cleanup()
	}
	if err := gz.Close(); err != nil {
		return cleanup()
	}
	if err := f.Close(); err != nil {
		a.fs.Remove(ctx, tmp)
		return err
	}
	return a.fs.Rename(ctx, tmp, filepath.Join(dir, name))
}

// ReadUnits calls f with the digest and content of each compilation unit
// stored in the units subdirectory of the index pack whose format key equals
// formatKey.  The concrete type of the value passed to f will be the same as
// the concrete type of the UnitType option that was passed to Open or Create.
// If no UnitType was specified, the value is a json.RawMessage.
//
// If f returns a non-nil error, no further compilations are read and the error
// is propagated back to the caller of ReadUnits.
func (a *Archive) ReadUnits(ctx context.Context, formatKey string, f func(string, interface{}) error) error {
	fss, err := a.fs.Glob(ctx, filepath.Join(filepath.Join(a.root, unitDir), "*"+unitSuffix))
	if err != nil {
		return err
	}
	for _, fs := range fss {
		digest := strings.TrimSuffix(filepath.Base(fs), unitSuffix)
		if err := a.ReadUnit(ctx, formatKey, digest, func(unit interface{}) error {
			return f(digest, unit)
		}); err != nil {
			return err
		}
	}
	return nil
}

// ReadUnit calls f with the compilation unit stored in the units subdirectory
// with the given formatKey and digest.  The concrete type of the value passed
// to f will be the same as the concrete type of the UnitType option that was
// passed to Open or Create.  If no UnitType was specified, the value is a
// json.RawMessage.
//
// If f returns a non-nil error, it is returned.
func (a *Archive) ReadUnit(ctx context.Context, formatKey, digest string, f func(interface{}) error) error {
	data, err := a.readFile(ctx, filepath.Join(a.root, unitDir), digest+unitSuffix)
	if err != nil {
		return err
	}

	// Parse the unit wrapper, {"format": "kythe", "content": ...}
	var unit unitWrapper
	if err := json.Unmarshal(data, &unit); err != nil {
		return fmt.Errorf("error parsing unit: %v", err)
	}
	if unit.Format != formatKey {
		return nil
	}
	if len(unit.Content) == 0 {
		return errors.New("invalid compilation unit")
	}

	// Parse the content into the receiver's type.
	cu := reflect.New(a.unitType).Interface()
	if err := json.Unmarshal(unit.Content, cu); err != nil {
		return fmt.Errorf("error parsing content: %v", err)
	}
	if err := f(cu); err != nil {
		return err
	}
	return nil
}

// ReadFile reads and returns the file contents corresponding to the given
// hex-encoded SHA-256 digest.
func (a *Archive) ReadFile(ctx context.Context, digest string) ([]byte, error) {
	return a.readFile(ctx, filepath.Join(a.root, dataDir), digest+dataSuffix)
}

// WriteUnit writes the specified compilation unit to the units/ subdirectory
// of the index pack, using the specified format key.  Returns the resulting
// filename, whether or not there is an error in writing the file, as long as
// marshaling succeeded.
func (a *Archive) WriteUnit(ctx context.Context, formatKey string, cu interface{}) (string, error) {
	// Convert the compilation unit into JSON.
	content, err := json.Marshal(cu)
	if err != nil {
		return "", fmt.Errorf("error marshaling content: %v", err)
	}

	// Pack the resulting message into a compilation wrapper for output.
	data, err := json.Marshal(&unitWrapper{
		Format:  formatKey,
		Content: content,
	})
	if err != nil {
		return "", fmt.Errorf("error marshaling unit: %v", err)
	}
	name := hexDigest(data) + unitSuffix
	return name, a.writeFile(ctx, filepath.Join(a.root, unitDir), name, data)
}

// FileExists determines whether a file with the given digest exists.
func (a *Archive) FileExists(ctx context.Context, digest string) (bool, error) {
	path := filepath.Join(a.root, dataDir, digest+dataSuffix)
	_, err := a.fs.Stat(ctx, path)
	if os.IsNotExist(err) {
		return false, nil
	}
	return err == nil, err
}

// WriteFile writes the specified file contents to the files/ subdirectory of
// the index pack.  Returns the resulting filename, whether or not there is an
// error in writing the file.
func (a *Archive) WriteFile(ctx context.Context, data []byte) (string, error) {
	name := hexDigest(data) + dataSuffix
	return name, a.writeFile(ctx, filepath.Join(a.root, dataDir), name, data)
}

// Root returns the root path of the archive.
func (a *Archive) Root() string { return a.root }

func hexDigest(data []byte) string {
	h := sha256.New()
	h.Write(data)
	return hex.EncodeToString(h.Sum(nil))
}
