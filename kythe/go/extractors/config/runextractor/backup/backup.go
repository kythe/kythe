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
//    tmp, err := backup.Save(someFile)
//    if err != nil {
//       return fmt.Errorf("backing up %q: %v", somePath, err)
//    }
//    defer tmp.Release()
//    // ... do real work ...
//    tmp.Restore()
package backup

import (
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
)

// A File is a simple holder for a backup of some file that supports Recovery
// to restore the original version.
type File struct {
	orig, tmp string // file paths
}

// New backs a given file up to an adjacent temporary file.
func New(orig string) (*File, error) {
	f, err := os.Open(orig)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	dir, base := filepath.Split(orig)
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

func (f *File) Restore() error {
	if err := os.Rename(f.tmp, f.orig); err != nil {
		return err
	}
	f.tmp = ""
	return nil
}

func (f *File) Release() {
	if f.tmp != "" {
		if err := os.Remove(f.tmp); err != nil {
			log.Printf("Warning: removing backup of %q failed: %v", f.orig, err)
		}
	}
}
