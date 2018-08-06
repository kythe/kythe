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
package backup

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
)

// Backup copies a given file to a temp file, returning the temp file name.
func Backup(configFile string) (tfname string, oerr error) {
	// Copy over the build file temporarily so we can undo any hacks.
	bf, err := os.Open(configFile)
	if err != nil {
		return "", fmt.Errorf("opening config file %s", configFile)
	}
	defer bf.Close()
	tf, err := ioutil.TempFile("", "tmp-build-file")
	if err != nil {
		return "", fmt.Errorf("opening temp file")
	}
	defer func() {
		oerr := bf.Close()
		if err == nil {
			err = oerr
		}
	}()
	_, err = io.Copy(tf, bf)
	return tf.Name(), err
}

// Restore copies a given temp file back to the original file location.
// TODO(danielmoy): consider not blindly copying back with 644 permissions.
func Restore(configFile string, tempFile string) (err error) {
	tf, err := os.Open(tempFile)
	if err != nil {
		return fmt.Errorf("opening %s: %v", tempFile, err)
	}
	defer tf.Close()
	bf, err := os.OpenFile(configFile, os.O_RDWR|os.O_CREATE, 0644)
	if err != nil {
		return fmt.Errorf("opening %s: %v", configFile, err)
	}
	defer func() {
		oerr := bf.Close()
		if err == nil {
			err = oerr
		}
	}()
	_, err = io.Copy(bf, tf)
	if err != nil {
		return fmt.Errorf("copying: %v", err)
	}

	return
}
