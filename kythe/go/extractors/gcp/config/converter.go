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
	cloudbuild "google.golang.org/api/cloudbuild/v1"
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

// KytheToYAML takes an input file, parses it as a JSON defined
// kythe.proto.extraction.RepoConfig, and converts it to YAML.
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

// KytheToBuild takes a kythe.proto.extraction.RepoConfig and generates the
// necessary cloudbuild.Build with BuildSteps for running kythe extraction.
func KytheToBuild(conf rpb.Config) (cloudbuild.Build, error) {
	build := cloudbuild.Build{}
	initializeArtifacts(&build)
	build.Artifacts.Objects.Location = fmt.Sprintf("gs://%s/", outputGsBucket)
	build.Artifacts.Objects.Paths = append(build.Artifacts.Objects.Paths, path.Join(outputDirectory, outputFilePattern))
	build.Steps = append(build.Steps, commonSteps()...)

	if len(conf.Extractions) == 0 {
		return build, fmt.Errorf("config has no extraction specified")
	} else if len(conf.Extractions) > 1 {
		return build, fmt.Errorf("we don't yet support multiple extraction in a single repo")
	}

	hints := conf.Extractions[0]
	bs := hints.BuildSystem

	// TODO(danielmoy): when we support bazel, we probably want to pull out the
	// switch cases here into something a bit easier to read.
	// I imagine something like a proper interface that each build system can
	// override, which would provide things like artifactPaths(), steps().
	switch bs {
	case rpb.BuildSystem_MAVEN:
		build.Steps = append(build.Steps, javaArtifactsStep())
		build.Steps = append(build.Steps, mavenStep(hints))
		build.Artifacts.Objects.Paths = append(build.Artifacts.Objects.Paths, path.Join(outputDirectory, "javac-extractor.err"))
	case rpb.BuildSystem_GRADLE:
		//build.Steps = append(build.Steps, GradleSteps())
		build.Artifacts.Objects.Paths = append(build.Artifacts.Objects.Paths, path.Join(outputDirectory, "javac-extractor.err"))
	default:
		return build, fmt.Errorf("unsupported build system %s", bs)
	}

	return build, nil
}

func readConfigFile(input string) (rpb.Config, error) {
	var conf rpb.Config
	file, err := os.Open(input)
	if err != nil {
		return conf, fmt.Errorf("opening input file %s: %v", input, err)
	}
	if err := jsonpb.Unmarshal(file, &conf); err != nil {
		return conf, fmt.Errorf("parsing json file %s: %v", input, err)
	}
	return conf, nil
}

func initializeArtifacts(build *cloudbuild.Build) {
	build.Artifacts = &cloudbuild.Artifacts{}
	build.Artifacts.Objects = &cloudbuild.ArtifactObjects{}
}
