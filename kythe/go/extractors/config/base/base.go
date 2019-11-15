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

// Package base and its subpackages define default config data for different
// builders (cmake, gradle, maven).
package base // import "kythe.io/kythe/go/extractors/config/base"

import (
	"log"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/extractors/config/base/gradle"
	"kythe.io/kythe/go/extractors/config/base/mvn"
)

// DefaultConfig tries to find a valid base builder that works for a given repo
// and returns a json-encoded kythe.proto.ExtractionConfiguration that will work
// for simple repos using thatbuilder.
func DefaultConfig(repoDir string) (string, bool) {
	// TODO(#3060): There might be more than one match, in which case we need to
	// know what to do to combine separate extractors' output.
	for _, b := range supportedBuilders() {
		if b.validFor(repoDir) {
			return b.DefaultConfig, true
		}
	}
	return "", false
}

type builder struct {
	Name string
	// BuildFile is the name pattern of the default build file that this builder
	// uses.  For eaxmple 'BUILD' for bazel, or 'pom.xml' for maven.
	BuildFile string
	// DefaultConfig returns a reader that yields a valid
	// kythe.proto.ExtractionConfiguration which is fine-tuned to work for a
	// simple repo of the given builder.
	DefaultConfig string
}

// validFor returns whether or not a builder is potentially valid for a given
// repo.
func (b builder) validFor(repoDir string) bool {
	// TODO(danielmoy): This logic probably needs to hunt for config
	// files that are not in the top-level directory.
	buildPath := filepath.Join(repoDir, b.BuildFile)
	f, err := os.Open(buildPath)
	if os.IsNotExist(err) {
		return false
	} else if err != nil {
		log.Printf("Failed to open build file %s", buildPath)
		return false
	}
	// TODO(danielmoy): In the event that we need to inspect the build file
	// itself to modify the default config in some way (pull in dependencies,
	// etc), then this cannot simply be closed without looking at it.  But for
	// now, we just always use default config.
	defer f.Close()
	return true
}

// supportedBuilders returns all of the supported builders for Kythe extraction.
func supportedBuilders() []builder {
	return []builder{
		builder{
			Name:          "gradle",
			BuildFile:     "build.gradle",
			DefaultConfig: gradle.DefaultConfig,
		},
		builder{
			Name:          "maven",
			BuildFile:     "pom.xml",
			DefaultConfig: mvn.DefaultConfig,
		},
	}
}
