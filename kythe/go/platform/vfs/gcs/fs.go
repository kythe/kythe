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

	"kythe.io/kythe/go/platform/vfs"

	"golang.org/x/net/context"
	"google.golang.org/cloud/storage"
)

// fs implements a VFS backed by a bucket in Google Cloud Storage
type fs struct {
	client *storage.Client
	bucket *storage.BucketHandle
}

// NewFS creates a new VFS backed by the given Google Cloud Storage bucket.
func NewFS(ctx context.Context, bucket string) (vfs.Interface, error) {
	c, err := storage.NewClient(ctx)
	if err != nil {
		return nil, err
	}
	return &fs{
		client: c,
		bucket: c.Bucket(bucket),
	}, nil
}

type objInfo struct{ *storage.ObjectAttrs }

// These methods implement the os.FileInfo interface
func (o objInfo) Name() string       { return filepath.Base(o.Name()) }
func (o objInfo) Size() int64        { return int64(o.Size()) }
func (o objInfo) Mode() os.FileMode  { return os.ModePerm }
func (o objInfo) ModTime() time.Time { return time.Time{} }
func (o objInfo) IsDir() bool        { return false }
func (o objInfo) Sys() interface{}   { return o.ObjectAttrs }

// Stat implements part of the VFS interface.
func (s *fs) Stat(ctx context.Context, path string) (os.FileInfo, error) {
	attrs, err := s.bucket.Object(path).Attrs(ctx)
	if err != nil {
		return nil, err
	}
	return objInfo{attrs}, nil
}

// MkdirAll implements part of the VFS interface.
func (fs) MkdirAll(_ context.Context, path string, mode os.FileMode) error { return nil }

// Open implements part of the VFS interface.
func (s *fs) Open(ctx context.Context, path string) (io.ReadCloser, error) {
	return s.bucket.Object(path).NewReader(ctx)
}

// Create implements part of the VFS interface.
func (s *fs) Create(ctx context.Context, path string) (io.WriteCloser, error) {
	return s.bucket.Object(path).NewWriter(ctx), nil
}

// Rename implements part of the VFS interface.
func (s *fs) Rename(ctx context.Context, oldPath, newPath string) error {
	src, dst := s.bucket.Object(oldPath), s.bucket.Object(newPath)
	if _, err := src.CopyTo(ctx, dst, &storage.ObjectAttrs{
		ContentType: "application/octet-stream",
	}); err != nil {
		return fmt.Errorf("error copying file during rename: %v", err)
	}
	if err := src.Delete(ctx); err != nil {
		return fmt.Errorf("error deleting old file during rename: %v", err)
	}
	return nil
}

// Remove implements part of the VFS interface.
func (s *fs) Remove(ctx context.Context, path string) error {
	return s.bucket.Object(path).Delete(ctx)
}

// Glob implements part of the VFS interface.
func (s *fs) Glob(ctx context.Context, glob string) ([]string, error) {
	q := &storage.Query{
		Prefix: globLiteralPrefix(glob),
	}
	var paths []string
	for q != nil {
		objs, err := s.bucket.List(ctx, q)
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
