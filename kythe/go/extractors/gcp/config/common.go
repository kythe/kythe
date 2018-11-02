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
	"kythe.io/kythe/go/extractors/constants"

	cloudbuild "google.golang.org/api/cloudbuild/v1"
)

const cloneStepID = "CLONE"

// commonSteps returns cloudbuild BuildSteps for copying a repo and creating
// an output directory.
//
// The BuildStep for the repo copy uses id cloneStepID, as described in
// https://cloud.google.com/cloud-build/docs/build-config#id, for any future
// steps that need to depend on the repo clone step.  The repo copy step puts
// the code into /workspace/code.
//
// The output directory is /workspace/out.
func commonSteps() []*cloudbuild.BuildStep {
	return []*cloudbuild.BuildStep{
		&cloudbuild.BuildStep{
			Name:    constants.GCRGitImage,
			Args:    []string{"git", repoName, "/workspace/code"},
			Id:      cloneStepID,
			WaitFor: []string{"-"},
		},
		&cloudbuild.BuildStep{
			Name:    "ubuntu",
			Args:    []string{"mkdir", "/workspace/out"},
			WaitFor: []string{"-"},
		},
	}
}

func preprocessorStep(build string) *cloudbuild.BuildStep {
	return &cloudbuild.BuildStep{
		Name:    constants.KytheBuildPreprocessorImage,
		Args:    []string{build},
		WaitFor: []string{cloneStepID},
	}
}

// TODO(#3095): This step needs to be configurable by the java version used for
// a given BuildTarget.
func javaArtifactsStep() *cloudbuild.BuildStep {
	return &cloudbuild.BuildStep{
		Name: constants.KytheJavacExtractorArtifactsImage,
		Volumes: []*cloudbuild.Volume{
			&cloudbuild.Volume{
				Name: javaVolumeName,
				Path: constants.DefaultExtractorsDir,
			},
		},
		WaitFor: []string{"-"},
	}
}
