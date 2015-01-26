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

package indexpack

import (
	"io"
	"os"
	"path/filepath"

	"golang.org/x/net/context"
)

// localFS implements the VFS interface using the standard Go library.
type localFS struct{}

func (localFS) Stat(_ context.Context, path string) (os.FileInfo, error) {
	return os.Stat(path)
}

func (localFS) MkdirAll(_ context.Context, path string, mode os.FileMode) error {
	return os.MkdirAll(path, mode)
}

func (localFS) Open(_ context.Context, path string) (io.ReadCloser, error) {
	return os.Open(path)
}

func (localFS) Create(_ context.Context, path string) (io.WriteCloser, error) {
	return os.Create(path)
}

func (localFS) Rename(_ context.Context, oldPath, newPath string) error {
	return os.Rename(oldPath, newPath)
}

func (localFS) Remove(_ context.Context, path string) error {
	return os.Remove(path)
}

func (localFS) Glob(_ context.Context, glob string) ([]string, error) {
	return filepath.Glob(glob)
}
