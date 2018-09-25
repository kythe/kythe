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
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/extractors/config/default/gradle"
	"kythe.io/kythe/go/extractors/config/default/mvn"

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
		// TODO(#3060): Depending on how we want to handle multiple types of
		// builders in a single repo, this may be a bad approach.
		// Specifically, we just greedily pick the first one we find and
		// ignore others.
		for _, b := range supportedBuilders() {
			r, ok := testBuilder(b, repoDir)
			if ok {
				return load(r)
			}
		}
		return nil, fmt.Errorf("failed to find a supported builder for repo %s", repoDir)
	} else if err != nil {
		return nil, fmt.Errorf("opening config file: %v", err)
	}

	defer f.Close()
	return load(f)
}

func supportedBuilders() []builderType {
	return []builderType{&mvn.Mvn{}, &gradle.Gradle{}}
}

// builderType gives info about a given builder.
type builderType interface {
	// Name is just a human-readable string describing the builder.
	Name() string
	// BuildFile should be the default build config file for the builder, like
	// 'BUILD' in bazel, 'pom.xml' in maven.
	BuildFile() string
	// DefaultConfig returns a reader that yields a valid
	// kythe.proto.ExtractionConfiguration which is fine-tuned to work for a
	// simple repo of the given BuilderType.
	DefaultConfig() io.Reader
}

// Note right now most of the matching logic lives out here in config_selector,
// under the assumption that figuring out what type of builder a given repo uses
// won't be that different based on the type of *builder*.  Instead, we assume
// that most of the logic will be based on the repo contents.  If that ends up
// being a faulty assumption, then BuildFile() above should probably changed to
// something like Matches(repoDir string), so that each individual builder type

func testBuilder(builder builderType, repoDir string) (io.Reader, bool) {
	// TODO(danielmoy): This logic probably needs to hunt for config
	// files that are not in the top-level directory.
	buildPath := filepath.Join(repoDir, builder.BuildFile())
	f, err := os.Open(buildPath)
	if os.IsNotExist(err) {
		return nil, false
	} else if err != nil {
		log.Printf("Failed to open build file %s", buildPath)
		return nil, false
	}
	// TODO(danielmoy): In the event that we need to inspect the build file
	// itself to modify the default config in some way (pull in dependencies,
	// etc), then this cannot simply be closed without looking at it.  But for
	// now, we just always use default config.
	defer f.Close()
	return builder.DefaultConfig(), true
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
