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
	"strconv"
	"strings"

	rpb "kythe.io/kythe/proto/repo_go_proto"

	"github.com/ghodss/yaml"
	"github.com/golang/protobuf/jsonpb"
	"google.golang.org/api/cloudbuild/v1"
)

// Constants that map input/output substitutions.
const (
	defaultCorpus   = "${_CORPUS}"
	defaultVersion  = "${_COMMIT}"
	outputGsBucket  = "${_BUCKET_NAME}"
	defaultRepoName = "${_REPO}"
)

// Constants ephemeral to a single kythe cloudbuild run.
const (
	outputDirectory = "/workspace/output"
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
		return nil, fmt.Errorf("we don't yet support multiple corpus extraction yet")
	}

	repo := conf.Repo
	if repo == "" {
		// Default to a $_{REPO} string replacement.
		// TODO(danielmoy): this kludge should be removed in favor of having
		// config generation be a separate process (something should generate a
		// qualified rpb.Config object, obviating the need to fall back to the
		// $_{VAR} replacements.
		repo = defaultRepoName
	}

	hints := conf.Extractions[0]
	if hints.Corpus == "" {
		// Default to a $_{CORPUS} string replacement.
		hints.Corpus = defaultCorpus
	}

	build := &cloudbuild.Build{
		Artifacts: &cloudbuild.Artifacts{
			Objects: &cloudbuild.ArtifactObjects{
				Location: fmt.Sprintf("gs://%s/%s/", outputGsBucket, hints.Corpus),
				Paths:    []string{path.Join(outputDirectory, outputFileName())},
			},
		},
		// TODO(danielmoy): this should probably also be a generator, or at least
		// if there is refactoring work done as described below to make steps
		// more granular, this will have to hook into that logic.
		Steps: commonSteps(repo),
		Tags:  []string{hints.Corpus},
	}

	g, err := generator(hints.BuildSystem)
	if err != nil {
		return nil, err
	}
	build.Tags = append(build.Tags, "extract_"+strings.ToLower(hints.BuildSystem.String()))

	build.Steps = append(build.Steps, g.preExtractSteps()...)

	targets := hints.Targets
	if len(targets) == 0 {
		targets = append(targets, g.defaultExtractionTarget())
	}
	for i, target := range targets {
		idSuffix := ""
		if len(targets) > 1 {
			idSuffix = strconv.Itoa(i)
		}
		build.Steps = append(build.Steps, g.extractSteps(hints.Corpus, target, idSuffix)...)
	}

	build.Steps = append(build.Steps, g.postExtractSteps(hints.Corpus)...)

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

// buildSystemElaborator encapsulates the logic for adding extra build steps for a
// specific type of build system (maven, gradle, etc).
// TODO(danielmoy): we will almost certainly need to support more fine-grained
// steps and/or artifacts.  For example now artifacts are prepended, we might
// need ones that are appended.  We might need build steps that occur before or
// directly after cloning, but before other common steps.
type buildSystemElaborator interface {
	// preExtractSteps is a list of cloudbuild steps to be done before
	// extraction starts, specific to this extractor type.
	preExtractSteps() []*cloudbuild.BuildStep
	// extractSteps is a list of cloudbuild steps to extract the given target.
	// The idSuffix is simply a unique identifier for this instance and target,
	// for use in coordinating paralleism if desired.
	extractSteps(corpus string, target *rpb.ExtractionTarget, idSuffix string) []*cloudbuild.BuildStep
	// postExtractSteps is a list of cloudbuild steps to be done after
	// extraction finishes.
	postExtractSteps(corpus string) []*cloudbuild.BuildStep
	// defaultExtractionTarget is the repo-relative location of the default build
	// configuration file for a repo of a given type.  For example for a bazel
	// repo it might just be root/BUILD, or a maven repo will have repo/pom.xml.
	defaultExtractionTarget() *rpb.ExtractionTarget
}

func generator(b rpb.BuildSystem) (buildSystemElaborator, error) {
	switch b {
	case rpb.BuildSystem_MAVEN:
		return &mavenGenerator{}, nil
	case rpb.BuildSystem_GRADLE:
		return &gradleGenerator{}, nil
	case rpb.BuildSystem_BAZEL:
		return &bazelGenerator{}, nil
	default:
		return nil, fmt.Errorf("unsupported build system %s", b)
	}
}

func outputFileName() string {
	return defaultVersion + ".kzip"
}
