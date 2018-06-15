/*
 * Copyright 2018 Google Inc. All rights reserved.
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

// Package config defines configuration settings for extracting compilation records from
// a repository using a non-bazel build system.
package config

import (
	"bytes"
	"fmt"
	"io"
	"io/ioutil"

	"bitbucket.org/creachadair/shell"
	"github.com/golang/protobuf/jsonpb"

	ecpb "kythe.io/kythe/proto/extraction_config_go_proto"
)

const (
	// DefaultRepoVolume The default volume name containing the repo.
	DefaultRepoVolume = "/repo"

	// DefaultOutputVolume The default volume name containing the extraction output.
	DefaultOutputVolume = "/out"

	// defaultBaseImage The default base image for extraction.
	defaultBaseImage = "debian:jessie"

	// outputVolumeEnv The EnvVar which will be configured pointing to the output.
	outputVolumeEnv = "KYTHE_OUTPUT_DIRECTORY"

	// repoVolumeEnv The EnvVar which will be configured pointing to the repo.
	repoVolumeEnv = "KYTHE_ROOT_DIRECTORY"
)

// Format the base configuration including the base image, volume hooks, and working dir.
var baseConfig = fmt.Sprintf(`
FROM %[1]s
VOLUME %[2]s
ENV %[3]s=%[2]s
VOLUME %[4]s
ENV %[5]s=%[4]s
WORKDIR %[2]s
`, defaultBaseImage, DefaultRepoVolume, repoVolumeEnv, DefaultOutputVolume, outputVolumeEnv)

// NewImage consumes the configuration data specified in
// extractionConfig utilizing it to generate a composite extraction Docker
// image tailored for the requirements necessary for successful extraction of
// the configuration's corresponding repository. Returns the contents of the
// Dockerfile for the generated composite image. The Dockerfile format is
// defined here: https://docs.docker.com/engine/reference/builder/
func NewImage(config *ecpb.ExtractionConfiguration, repoDir, outputDir string) ([]byte, error) {
	if repoDir == "" {
		repoDir = DefaultRepoVolume
	}
	if outputDir == "" {
		outputDir = DefaultOutputVolume
	}
	var buf bytes.Buffer

	// Format the FROM statements for the required images.
	for _, image := range config.RequiredImage {
		fmt.Fprintf(&buf, "FROM %s as %s\n", image.Uri, image.Name)
	}

	// Format the base configuration into the current config.
	fmt.Fprintf(&buf, `
FROM %[1]s
VOLUME %[2]s
ENV %[3]s=%[2]s
VOLUME %[4]s
ENV %[5]s=%[4]s
WORKDIR %[2]s
`, defaultBaseImage, repoDir, repoVolumeEnv, outputDir, outputVolumeEnv)

	// Format the COPY statements for the required images, (these must come after
	// the last FROM statement due to the way docker's multi-stage builds work).
	for _, image := range config.RequiredImage {
		for _, artifact := range image.CopySpec {
			if artifact.Source == "" {
				return nil, fmt.Errorf("missing image copy artifact source for required image: %s", image.Uri)
			}

			// attempt to retrieve the dest defaulting to the source path
			dest := artifact.Source
			if artifact.Destination != "" {
				dest = artifact.Destination
			}

			fmt.Fprintf(&buf, "COPY --from=%s %s %s\n", image.Name, artifact.Source, dest)
		}

		// process required environment variables for the image
		for _, env := range image.EnvVar {
			if env.Name == "" {
				return nil, fmt.Errorf("missing Image.EnvVar.Name")
			}

			if env.Value == "" {
				return nil, fmt.Errorf("missing Image.EnvVar.Value")
			}

			fmt.Fprintf(&buf, "ENV %s=%s\n", env.Name, env.Value)
		}
	}

	// iterate over the required RUN commands for the configuration
	for _, cmd := range config.RunCommand {
		fmt.Fprintf(&buf, "RUN %s ", cmd.Command)
		for i, arg := range cmd.Arg {
			fmt.Fprintf(&buf, shell.Quote(arg))
			if i < len(cmd.Arg)-1 {
				fmt.Fprintf(&buf, " ")
			}
		}
		fmt.Fprintln(&buf)
	}

	// process the required entry point for the configuration
	if len(config.EntryPoint) > 0 {
		fmt.Fprintf(&buf, "ENTRYPOINT [")
		for i, entryPointComponent := range config.EntryPoint {
			fmt.Fprintf(&buf, "\"%s\"", entryPointComponent)
			if i < len(config.EntryPoint)-1 {
				fmt.Fprintf(&buf, ", ")
			}
		}
		fmt.Fprintf(&buf, "]\n")
	}

	return buf.Bytes(), nil
}

// Load parses an extraction configuration from the specified reader.
func Load(r io.Reader) (*ecpb.ExtractionConfiguration, error) {
	// attempt to deserialize the extraction config
	extractionConfig := &ecpb.ExtractionConfiguration{}
	if err := jsonpb.Unmarshal(r, extractionConfig); err != nil {
		return nil, err
	}

	return extractionConfig, nil
}

// CreateImage uses the specified extraction configuration to generate a
// new extraction image, which is written to the specified output path.
func CreateImage(outputPath string, config *ecpb.ExtractionConfiguration, repoDir, extractionOutDir string) error {
	// attempt to generate a docker image from the specified config
	image, err := NewImage(config, repoDir, extractionOutDir)
	if err != nil {
		return fmt.Errorf("generating extraction image: %v", err)
	}

	// write the generated extraction image to the specified output file
	if err = ioutil.WriteFile(outputPath, image, 0444); err != nil {
		return fmt.Errorf("writing output: %v", err)
	}

	return nil
}
