/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

// Package backup is a simple library for backing up a config file and restoring
// it using a temporary file.
//
// Example usage:
//
//	tmp, err := backup.Save(someFile)
//	if err != nil {
//	   return fmt.Errorf("backing up %q: %v", somePath, err)
//	}
//	defer tmp.Release()
//	// ... do real work ...
//	tmp.Restore()
package backup // import "kythe.io/kythe/go/extractors/config/runextractor/backup"

import (
	"io"
	"io/ioutil"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/util/log"

	"github.com/google/go-cmp/cmp"
)

// A File records the locations of an original file and a backup copy of that
// file.  The Restore method replaces the contents of the original with the
// backup, overwriting any changes that were made since the backup was created.
type File struct {
	orig, tmp string // file paths
}

// New creates a backup copy of the specified file, located in the same
// directory. The caller should ensure the Release method is called when the
// backup is no longer needed, to clean up.
func New(orig string) (*File, error) {
	f, err := os.Open(orig)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	dir := filepath.Dir(orig)
	base := filepath.Base(orig)
	tf, err := ioutil.TempFile(dir, base+".bkup")
	if err != nil {
		return nil, err
	}
	_, err = io.Copy(tf, f)
	cerr := tf.Close()
	if err != nil {
		return nil, err
	} else if cerr != nil {
		return nil, cerr
	}
	return &File{orig, tf.Name()}, nil
}

// Restore puts the original version of the backed up file back in place.
func (f *File) Restore() error {
	if err := os.Rename(f.tmp, f.orig); err != nil {
		return err
	}
	f.tmp = ""
	return nil
}

// Release removes the temporary file copy if it hasn't already been moved.
func (f *File) Release() {
	if f.tmp != "" {
		if err := os.Remove(f.tmp); err != nil {
			log.Warningf("removing backup of %q failed: %v", f.orig, err)
		}
	}
}

// GetDiff compares the modified file against the original and returns a diff.
func (f *File) GetDiff() (string, error) {
	before, err := ioutil.ReadFile(f.tmp)
	if err != nil {
		return "", err
	}
	after, err := ioutil.ReadFile(f.orig)
	if err != nil {
		return "", err
	}
	return cmp.Diff(string(before), string(after)), nil
}
