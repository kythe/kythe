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

import (
	"path"

	"kythe.io/kythe/go/extractors/constants"
	rpb "kythe.io/kythe/proto/repo_go_proto"

	"google.golang.org/api/cloudbuild/v1"
)

type bazelGenerator struct{}

const defaultBazelTarget = "//..."

// preExtractSteps implements parts of buildSystemElaborator
func (b bazelGenerator) preExtractSteps() []*cloudbuild.BuildStep {
	return []*cloudbuild.BuildStep{}
}

// extractSteps implements parts of buildSystemElaborator
func (b bazelGenerator) extractSteps(corpus string, target *rpb.ExtractionTarget, _ string /* unused prefix */) []*cloudbuild.BuildStep {
	args := []string{
		"build",
		"-k",
		"--define",
		"kythe_corpus=${_CORPUS}",
	}

	// Default target to just //...
	if len(target.IndividualTargets) == 0 {
		args = append(args, defaultBazelTarget)
	} else {
		for _, buildTarget := range target.IndividualTargets {
			args = append(args, buildTarget)
		}
	}

	return []*cloudbuild.BuildStep{

		&cloudbuild.BuildStep{
			Name: constants.BazelImage,
			Args: args,
			Dir:  codeDirectory,
			Id:   extractStepID,
		},
	}
}

// postExtractSteps implements parts of buildSystemElaborator
func (b bazelGenerator) postExtractSteps(corpus string) []*cloudbuild.BuildStep {
	// kythe-public/bazel-extractor includes zipmerge already, because we won't
	// know where bazel deposits kzips ahead of time.
	return []*cloudbuild.BuildStep{
		&cloudbuild.BuildStep{
			Name: "ubuntu",
			Args: []string{
				"mv",
				path.Join(outputDirectory, "compilations.kzip"),
				path.Join(outputDirectory, outputFileName()),
			},
			Id: renameStepID,
		},
	}
}

// defaultConfigFile implements parts of buildSystemElaborator
func (b bazelGenerator) defaultExtractionTarget() *rpb.ExtractionTarget {
	return &rpb.ExtractionTarget{
		Path: "build.gradle",
	}
}
