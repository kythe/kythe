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

// Package config contains logic for converting
// kythe.proto.extraction.RepoConfig to cloudbuild.yaml format as specified by
// https://cloud.google.com/cloud-build/docs/build-config.
package config

import (
	"fmt"
	"os"
	"path"

	rpb "kythe.io/kythe/proto/repo_go_proto"

	"github.com/ghodss/yaml"
	"github.com/golang/protobuf/jsonpb"
	"google.golang.org/api/cloudbuild/v1"
)

// Constants that map input/output substitutions.
const (
	corpus            = "${_CORPUS}"
	outputFilePattern = "${_OUTPUT_KZIP_NAME}"
	outputGsBucket    = "${_OUTPUT_GS_BUCKET}"
	repoName          = "${_REPO_NAME}"
)

// Constants ephemeral to a single kythe cloudbuild run.
const (
	outputDirectory = "/workspace/out"
	codeDirectory   = "/workspace/code"
	javaVolumeName  = "kythe_extractors"
)

// KytheToYAML takes an input JSON file defined in
// kythe.proto.extraction.RepoConfig format, and returns it as marshalled YAML.
func KytheToYAML(input string) ([]byte, error) {
	configProto, err := readConfigFile(input)
	if err != nil {
		return nil, fmt.Errorf("reading config file %s: %v", input, configProto)
	}
	build, err := KytheToBuild(configProto)
	if err != nil {
		return nil, fmt.Errorf("converting cloudbuild.Build: %v", err)
	}
	json, err := build.MarshalJSON()
	if err != nil {
		return nil, fmt.Errorf("marshalling cloudbuild.Build to JSON: %v", err)
	}
	return yaml.JSONToYAML(json)
}

// KytheToBuild takes a kythe.proto.extraction.RepoConfig and returns the
// necessary cloudbuild.Build with BuildSteps for running kythe extraction.
func KytheToBuild(conf *rpb.Config) (*cloudbuild.Build, error) {
	if len(conf.Extractions) == 0 {
		return nil, fmt.Errorf("config has no extraction specified")
	} else if len(conf.Extractions) > 1 {
		return nil, fmt.Errorf("we don't yet support multiple extraction in a single repo")
	}

	build := &cloudbuild.Build{
		Artifacts: &cloudbuild.Artifacts{
			Objects: &cloudbuild.ArtifactObjects{
				Location: fmt.Sprintf("gs://%s/", outputGsBucket),
				Paths:    []string{path.Join(outputDirectory, outputFilePattern)},
			},
		},
		// TODO(danielmoy): this should probably also be a generator, or at least
		// if there is refactoring work done as described below to make steps
		// more granular, this will have to hook into that logic.
		Steps: commonSteps(),
	}

	hints := conf.Extractions[0]

	g, err := generator(hints.BuildSystem)
	if err != nil {
		return nil, err
	}
	build.Artifacts.Objects.Paths = append(g.preArtifacts(), build.Artifacts.Objects.Paths...)
	build.Steps = append(build.Steps, g.steps(hints)...)
	return build, nil
}

func readConfigFile(input string) (*rpb.Config, error) {
	conf := &rpb.Config{}
	file, err := os.Open(input)
	if err != nil {
		return nil, fmt.Errorf("opening input file %s: %v", input, err)
	}
	defer file.Close()
	if err := jsonpb.Unmarshal(file, conf); err != nil {
		return nil, fmt.Errorf("parsing json file %s: %v", input, err)
	}
	return conf, nil
}

// buildStepGenerator encapsulates the logic for adding extra build steps for a
// specific type of build system (maven, gradle, etc).
// TODO(danielmoy): we will almost certainly need to support more fine-grained
// steps and/or artifacts.  For example now artifacts are prepended, we might
// need ones that are appended.  We might need build steps that occur before or
// directly after cloning, but before other common steps.
type buildStepGenerator interface {
	// preArtifacts should be prepended to the list of artifacts returned from
	// the cloudbuild invocation.
	preArtifacts() []string
	// steps is a list of cloudbuild steps specific for this builder for
	// extraction.
	steps(conf *rpb.ExtractionHint) []*cloudbuild.BuildStep
}

func generator(b rpb.BuildSystem) (buildStepGenerator, error) {
	switch b {
	case rpb.BuildSystem_MAVEN:
		return &mavenGenerator{}, nil
	case rpb.BuildSystem_GRADLE:
		return &gradleGenerator{}, nil
	//case rpb.BuildSystem_BAZEL:
	//		return bazelSteps
	default:
		return nil, fmt.Errorf("unsupported build system %s", b)
	}
}
