/*
 * Copyright 2017 Google Inc. All rights reserved.
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

package languageserver

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
)

const settingsFile = ".kythe_settings.json"

// VNameConfig contains pathmap patterns for VName components
type VNameConfig struct {
	Corpus string `json:"corpus"`
	Path   string `json:"path"`
	Root   string `json:"root"`
}

// MappingConfig contains pathmap patterns for local paths and VNames
type MappingConfig struct {
	Local string      `json:"local"`
	VName VNameConfig `json:"vname"`
}

// Settings contains the user configuration required for the server to
// communicate properly with its XRefClient
type Settings struct {
	Mappings []MappingConfig `json:"mappings"`
}

// Attempts to read and parse the settings file at the given path.
// If this fails the default settings object will be returned along
// with an error.
func unmarshalSettingsFile(p string) (Settings, error) {
	// Default settings
	var m []MappingConfig
	s := Settings{
		Mappings: m,
	}

	dat, err := ioutil.ReadFile(p)
	if err != nil {
		return s, err
	}

	if err := json.Unmarshal(dat, &s); err != nil {
		return s, err
	}

	return s, nil
}

func findRoot(p string) (string, error) {
	for {
		file := filepath.Join(p, settingsFile)
		_, err := os.Stat(file)
		if err == nil {
			return p, nil
		}
		if !os.IsNotExist(err) {
			return "", fmt.Errorf("error searching for settings file: %v", err)
		}
		parent := filepath.Join(p, "..")
		if parent == p {
			return "", fmt.Errorf("%s file not found in hierarchy", settingsFile)
		}
		p = parent
	}
}
