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

// Package vfs defines a generic file system interface commonly used by Kythe
// libraries.
package vfs

import (
	"errors"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"

	"golang.org/x/net/context"
)

// ErrNotSupported is returned for all unsupported VFS operations.
var ErrNotSupported = errors.New("operation not supported")

// Interface is a virtual file system interface for reading and writing files.
// It is used to wrap the normal os package functions so that other file storage
// implementations be used in lieu.  For instance, there could be
// implementations for cloud storage back-ends or databases.  Depending on the
// implementation, the Writer methods can be unsupported and always return
// ErrNotSupported.
type Interface interface {
	Reader
	Writer
}

// Reader is a virtual file system interface for reading files.
type Reader interface {
	// Stat returns file status information for path, as os.Stat.
	Stat(ctx context.Context, path string) (os.FileInfo, error)

	// Open opens an existing file for reading, as os.Open.
	Open(ctx context.Context, path string) (io.ReadCloser, error)

	// Glob returns all the paths matching the specified glob pattern, as
	// filepath.Glob.
	Glob(ctx context.Context, glob string) ([]string, error)
}

// Writer is a virtual file system interface for writing files.
type Writer interface {
	// MkdirAll recursively creates the specified directory path with the given
	// permissions, as os.MkdirAll.
	MkdirAll(ctx context.Context, path string, mode os.FileMode) error

	// Create creates a new file for writing, as os.Create.
	Create(ctx context.Context, path string) (io.WriteCloser, error)

	// Rename renames oldPath to newPath, as os.Rename, overwriting newPath if
	// it exists.
	Rename(ctx context.Context, oldPath, newPath string) error

	// Remove deletes the file specified by path, as os.Remove.
	Remove(ctx context.Context, path string) error
}

// Default is the global default VFS used by Kythe libraries that wish to access
// the file system.  This is usually the LocalFS and should only be changed in
// very specialized cases (i.e. don't change it).
var Default Interface = LocalFS{}

// ReadFile is the equivalent of ioutil.ReadFile using the Default VFS.
func ReadFile(ctx context.Context, filename string) ([]byte, error) {
	f, err := Open(ctx, filename)
	if err != nil {
		return nil, err
	}
	defer f.Close() // ignore errors
	return ioutil.ReadAll(f)
}

// Stat returns file status information for path, using the Default VFS.
func Stat(ctx context.Context, path string) (os.FileInfo, error) { return Default.Stat(ctx, path) }

// MkdirAll recursively creates the specified directory path with the given
// permissions, using the Default VFS.
func MkdirAll(ctx context.Context, path string, mode os.FileMode) error {
	return Default.MkdirAll(ctx, path, mode)
}

// Open opens an existing file for reading, using the Default VFS.
func Open(ctx context.Context, path string) (io.ReadCloser, error) { return Default.Open(ctx, path) }

// Create creates a new file for writing, using the Default VFS.
func Create(ctx context.Context, path string) (io.WriteCloser, error) {
	return Default.Create(ctx, path)
}

// Rename renames oldPath to newPath, using the Default VFS, overwriting newPath
// if it exists.
func Rename(ctx context.Context, oldPath, newPath string) error {
	return Default.Rename(ctx, oldPath, newPath)
}

// Remove deletes the file specified by path, using the Default VFS.
func Remove(ctx context.Context, path string) error { return Default.Remove(ctx, path) }

// Glob returns all the paths matching the specified glob pattern, using the
// Default VFS.
func Glob(ctx context.Context, glob string) ([]string, error) { return Default.Glob(ctx, glob) }

// LocalFS implements the VFS interface using the standard Go library.
type LocalFS struct{}

// Stat implements part of the VFS interface.
func (LocalFS) Stat(_ context.Context, path string) (os.FileInfo, error) {
	return os.Stat(path)
}

// MkdirAll implements part of the VFS interface.
func (LocalFS) MkdirAll(_ context.Context, path string, mode os.FileMode) error {
	return os.MkdirAll(path, mode)
}

// Open implements part of the VFS interface.
func (LocalFS) Open(_ context.Context, path string) (io.ReadCloser, error) {
	if path == "-" {
		return ioutil.NopCloser(os.Stdin), nil
	}
	return os.Open(path)
}

// Create implements part of the VFS interface.
func (LocalFS) Create(_ context.Context, path string) (io.WriteCloser, error) {
	return os.Create(path)
}

// Rename implements part of the VFS interface.
func (LocalFS) Rename(_ context.Context, oldPath, newPath string) error {
	return os.Rename(oldPath, newPath)
}

// Remove implements part of the VFS interface.
func (LocalFS) Remove(_ context.Context, path string) error {
	return os.Remove(path)
}

// Glob implements part of the VFS interface.
func (LocalFS) Glob(_ context.Context, glob string) ([]string, error) {
	return filepath.Glob(glob)
}

// UnsupportedWriter implements the Writer interface methods with stubs that
// always return ErrNotSupported.
type UnsupportedWriter struct{ Reader }

// Create implements part of Writer interface.  It is not supported.
func (UnsupportedWriter) Create(_ context.Context, _ string) (io.WriteCloser, error) {
	return nil, ErrNotSupported
}

// MkdirAll implements part of Writer interface.  It is not supported.
func (UnsupportedWriter) MkdirAll(_ context.Context, _ string, _ os.FileMode) error {
	return ErrNotSupported
}

// Rename implements part of Writer interface.  It is not supported.
func (UnsupportedWriter) Rename(_ context.Context, _, _ string) error { return ErrNotSupported }

// Remove implements part of Writer interface.  It is not supported.
func (UnsupportedWriter) Remove(_ context.Context, _ string) error { return ErrNotSupported }
