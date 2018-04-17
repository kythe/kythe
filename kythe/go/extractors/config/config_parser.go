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

	"bitbucket.org/creachadair/shell"

	ecpb "kythe.io/kythe/proto/extraction_config_go_proto"
)

const (
	// DefaultBaseImage The default base image for extraction.
	defaultBaseImage = "debian:jessie"

	// DefaultRepoVolume The default volume name containing the repo.
	defaultRepoVolume = "/repo"

	// RepoVolumeEnv The EnvVar which will be configured pointing to the repo.
	repoVolumeEnv = "KYTHE_ROOT_DIRECTORY"

	// DefaultOutputVolume The default volume name containing the extraction output.
	defaultOutputVolume = "/out"

	// OutputVolumeEnv The EnvVar which will be configured pointing to the output.
	outputVolumeEnv = "KYTHE_OUTPUT_DIRECTORY"
)

// Format the base configuration including the base image, volume hooks, and working dir.
var baseConfig = fmt.Sprintf(`
FROM %[1]s
VOLUME %[2]s
ENV %[3]s=%[2]s
VOLUME %[4]s
ENV %[5]s=%[4]s
WORKDIR %[2]s
`, defaultBaseImage, defaultRepoVolume, repoVolumeEnv, defaultOutputVolume, outputVolumeEnv)

// NewExtractionImage consumes the configuration data specified in
// extractionConfig utilizing it to generate a composite extraction Docker
// image tailored for the requirements necessary for successful extraction of
// the configuration's corresponding repository. Returns the contents of the
// Dockerfile for the generated composite image. The Dockerfile format is
// defined here: https://docs.docker.com/engine/reference/builder/
func NewExtractionImage(config *ecpb.ExtractionConfiguration) ([]byte, error) {
	var buf bytes.Buffer

	// Format the FROM statements for the required images.
	for _, image := range config.RequiredImage {
		fmt.Fprintf(&buf, "FROM %s as %s\n", image.Uri, image.Name)
	}

	// Format the base configuration into the current config.
	fmt.Fprintf(&buf, baseConfig)

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

		// process the required entry point for the image
		if len(image.EntryPoint) > 0 {
			fmt.Fprintf(&buf, "ENTRYPOINT [")
			for i, entryPointComponent := range image.EntryPoint {
				fmt.Fprintf(&buf, "\"%s\"", entryPointComponent)
				if i < len(image.EntryPoint)-1 {
					fmt.Fprintf(&buf, ", ")
				}
			}
			fmt.Fprintf(&buf, "]\n")
		}
	}

	// iterate over the required RUN commands for the configuration
	for _, cmd := range config.RunCommand {
		fmt.Fprintf(&buf, "RUN %s ", cmd.Command)
		if len(cmd.Arg) == 0 {
			return nil, fmt.Errorf("missing Image.RunCommand.Arg")
		}

		for i, arg := range cmd.Arg {
			fmt.Fprintf(&buf, shell.Quote(arg))
			if i < len(cmd.Arg)-1 {
				fmt.Fprintf(&buf, " ")
			}
		}
	}

	return buf.Bytes(), nil
}
