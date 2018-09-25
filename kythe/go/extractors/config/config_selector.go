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

package config

// This subpart of package config handles getting a
// kythe.proto.ExtractionConfiguration either from a file or the default config.

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/config/base"

	"github.com/golang/protobuf/jsonpb"

	ecpb "kythe.io/kythe/proto/extraction_config_go_proto"
)

func findConfig(configPath, repoDir string) (*ecpb.ExtractionConfiguration, error) {
	// if a config was passed in, use the specified config, otherwise go
	// hunt for one in the repository.
	if configPath == "" {
		// otherwise, use a Kythe config within the repo (if it exists)
		configPath = filepath.Join(repoDir, kytheExtractionConfigFile)
	}

	f, err := os.Open(configPath)
	if os.IsNotExist(err) {
		if defaultConfig, ok := base.DefaultConfig(repoDir); ok {
			return load(strings.NewReader(defaultConfig))
		}
		return nil, fmt.Errorf("failed to find a supported builder for repo %s", repoDir)
	} else if err != nil {
		return nil, fmt.Errorf("opening config file: %v", err)
	}

	defer f.Close()
	return load(f)
}

// load parses an extraction configuration from the specified reader.
func load(r io.Reader) (*ecpb.ExtractionConfiguration, error) {
	// attempt to deserialize the extraction config
	extractionConfig := &ecpb.ExtractionConfiguration{}
	if err := jsonpb.Unmarshal(r, extractionConfig); err != nil {
		return nil, err
	}

	return extractionConfig, nil
}
