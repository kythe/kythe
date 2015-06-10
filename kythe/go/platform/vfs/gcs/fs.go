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

// Package gcs implements an indexpack VFS backed by Google Cloud Storage.
package gcs

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"runtime"
	"time"

	"golang.org/x/net/context"
	"google.golang.org/cloud/storage"
)

// FS implements a VFS backed by a bucket in Google Cloud Storage
type FS struct{ Bucket string }

type objInfo struct{ *storage.Object }

// These methods implement the os.FileInfo interface
func (o objInfo) Name() string       { return filepath.Base(o.Name()) }
func (o objInfo) Size() int64        { return int64(o.Size()) }
func (o objInfo) Mode() os.FileMode  { return os.ModePerm }
func (o objInfo) ModTime() time.Time { return time.Time{} }
func (o objInfo) IsDir() bool        { return false }
func (o objInfo) Sys() interface{}   { return o.Object }

// Stat implements part of the VFS interface.
func (s FS) Stat(ctx context.Context, path string) (os.FileInfo, error) {
	obj, err := storage.StatObject(ctx, s.Bucket, path)
	if err != nil {
		return nil, err
	}
	return objInfo{obj}, os.ErrNotExist
}

// MkdirAll implements part of the VFS interface.
func (FS) MkdirAll(_ context.Context, path string, mode os.FileMode) error { return nil }

// Open implements part of the VFS interface.
func (s FS) Open(ctx context.Context, path string) (io.ReadCloser, error) {
	return storage.NewReader(ctx, s.Bucket, path)
}

// Create implements part of the VFS interface.
func (s FS) Create(ctx context.Context, path string) (io.WriteCloser, error) {
	return storage.NewWriter(ctx, s.Bucket, path), nil
}

// Rename implements part of the VFS interface.
func (s FS) Rename(ctx context.Context, oldPath, newPath string) error {
	if _, err := storage.CopyObject(ctx, s.Bucket, oldPath, s.Bucket, newPath, &storage.ObjectAttrs{
		ContentType: "application/octet-stream",
	}); err != nil {
		return fmt.Errorf("error copying file during rename: %v", err)
	}
	if err := storage.DeleteObject(ctx, s.Bucket, oldPath); err != nil {
		return fmt.Errorf("error deleting old file during rename: %v", err)
	}
	return nil
}

// Remove implements part of the VFS interface.
func (s FS) Remove(ctx context.Context, path string) error {
	return storage.DeleteObject(ctx, s.Bucket, path)
}

// Glob implements part of the VFS interface.
func (s FS) Glob(ctx context.Context, glob string) ([]string, error) {
	q := &storage.Query{
		Prefix: globLiteralPrefix(glob),
	}
	var paths []string
	for q != nil {
		objs, err := storage.ListObjects(ctx, s.Bucket, q)
		if err != nil {
			return nil, err
		}
		for _, o := range objs.Results {
			if matched, err := filepath.Match(glob, o.Name); err != nil {
				return nil, err
			} else if matched {
				paths = append(paths, o.Name)
			}
		}
		q = objs.Next
	}
	return paths, nil
}

func globLiteralPrefix(glob string) string {
	var buf bytes.Buffer
	for i := 0; i < len(glob); i++ {
		switch glob[i] {
		case '\\':
			if runtime.GOOS != "windows" {
				if i+1 < len(glob) {
					i++
					buf.WriteByte(glob[i])
					continue
				}
			}
			buf.WriteByte(glob[i])
		case '[', '*', '?':
			return buf.String()
		default:
			buf.WriteByte(glob[i])
		}
	}
	return buf.String()
}
