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

package archive

import (
	"archive/tar"
	"archive/zip"
	"bytes"
	"compress/gzip"
	"errors"
	"io"
	"io/ioutil"
	"os"
	"reflect"
	"testing"

	"kythe.io/kythe/go/util/log"
)

func newTar(files map[string]string) *bytes.Buffer {
	buf := bytes.NewBuffer(nil)
	w := tar.NewWriter(buf)
	for name, data := range files {
		if err := w.WriteHeader(&tar.Header{
			Name: name,
			Size: int64(len(data)),
		}); err != nil {
			log.Fatal("NewHeader: ", err)
		}
		if _, err := w.Write([]byte(data)); err != nil {
			log.Fatal("Write: ", err)
		}
	}
	if err := w.Close(); err != nil {
		log.Fatal("Close: ", err)
	}
	return buf
}

func newZIP(files map[string]string) *bytes.Buffer {
	buf := bytes.NewBuffer(nil)
	w := zip.NewWriter(buf)
	for name, data := range files {
		f, err := w.Create(name)
		if err != nil {
			log.Fatal("Create: ", err)
		} else if _, err := f.Write([]byte(data)); err != nil {
			log.Fatal("Write: ", err)
		}
	}
	if err := w.Close(); err != nil {
		log.Fatal("Close: ", err)
	}
	return buf
}

func TestScanNothing(t *testing.T) {
	var ignore *os.File

	// These paths should not be recognized as archives.
	for _, path := range []string{"foo", "foo/bar", "foo.gz", "foo.bz2", "a/b/foo.zippy"} {
		err := Scan(ignore, path, func(name string, err error, rc io.Reader) error {
			t.Errorf("ScanFunc was called with path=%q, err=%v, rc=%v", name, err, rc)
			return nil
		})
		if err != ErrNotArchive {
			t.Errorf("Scan(%q): got error %v, want %v", path, err, ErrNotArchive)
		}
	}
}

// byteFile implements the File interface around a *bytes.Reader.  It adds a
// no-op Close method to complete the interface.
type byteFile struct{ *bytes.Reader }

func (b byteFile) Close() error { return nil }

func TestScanErrors(t *testing.T) {
	file := byteFile{
		Reader: bytes.NewReader(newZIP(map[string]string{
			"this is not my hat": "♡",
		}).Bytes()),
	}

	// Verify that an error returned by the callback is propagated.
	want := errors.New("everything you know is wrong")
	var got string
	err := Scan(file, "fail.zip", func(name string, err error, r io.Reader) error {
		got = name
		return want
	})
	if err != want {
		t.Errorf("Scan(fail.zip): got error %v, want %v", err, want)
	}
	if want := "this is not my hat"; got != want {
		t.Errorf("Scan(fail.zip): got name %q, want %q", got, want)
	}
}

func TestScanZIP(t *testing.T) {
	files := map[string]string{
		"devtools/grok/OWNERS": "stevey",
		"file/base/file.h":     "#include <stdio.h>",
		"pyglib.app.py":        "from __future__ import golang",
	}
	data := newZIP(files).Bytes()

	// Each of these paths should be recognized as a ZIP file.
	for _, path := range []string{"foo.zip", "bar.ZIP", "foo/bar.zip", "foo/bar/baz.plotz.zip", "ack/bar.jar"} {
		file := byteFile{bytes.NewReader(data)}
		got := make(map[string]string)

		err := Scan(file, path, func(name string, err error, r io.Reader) error {
			if err != nil {
				return err
			}
			data, err := ioutil.ReadAll(r)
			if err != nil {
				return err
			}
			got[name] = string(data)
			return nil
		})
		if err != nil {
			t.Errorf("Scan(%q): unexpected error: %v", path, err)
		}

		if !reflect.DeepEqual(got, files) {
			t.Errorf("Scan(%q):\n got %+v\nwant %+v", path, got, files)
		}
	}
}

func TestScanTar(t *testing.T) {
	files := map[string]string{
		"mapreduce/public/mapreduce.protodevel": `syntax = "proto3";`,
		"file/base/BUILD":                       `cc_library(name = "recordio", ...)`,
		"README.txt":                            "Lasciate ogni speranza, voi ch'entrate",
		"third_party/README.md":                 `This directory is where your gross hacks go`,
	}
	data := newTar(files).Bytes()
	gzdata := gzipData(data)

	// Each of these paths should be recognized as a tar file.
	tests := []struct {
		path string
		data []byte
	}{
		{"plain.tar", data},
		{"foo/bar/plain.old.tar", data},
		{"eat-a-tar.tar.gz", gzdata},
		{"doom/and/chaos.tgz", gzdata},
	}
	for _, test := range tests {
		file := byteFile{bytes.NewReader(test.data)}
		got := make(map[string]string)

		err := Scan(file, test.path, func(name string, err error, r io.Reader) error {
			if err != nil {
				return err
			}
			data, err := ioutil.ReadAll(r)
			if err != nil {
				return err
			}
			got[name] = string(data)
			return nil
		})
		if err != nil {
			t.Errorf("Scan(%q): unexpected error: %v", test.path, err)
		}

		if !reflect.DeepEqual(got, files) {
			t.Errorf("Scan(%q):\n got %+v\nwant %+v", test.path, got, files)
		}
	}
}

func gzipData(data []byte) []byte {
	var buf bytes.Buffer
	w := gzip.NewWriter(&buf)
	if _, err := w.Write(data); err != nil {
		log.Fatal("Write: ", err)
	} else if err := w.Close(); err != nil {
		log.Fatal("Close: ", err)
	}
	return buf.Bytes()
}
