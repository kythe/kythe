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
	"fmt"
	"path"
	"strconv"

	"kythe.io/kythe/go/extractors/constants"

	"google.golang.org/api/cloudbuild/v1"
)

const cloneStepID = "CLONE"
const checkoutStepID = "CHECKOUT"
const javaArtifactsID = "JAVA-ARTIFACTS"
const preStepID = "PREPROCESS"
const extractStepID = "EXTRACT"

// commonSteps returns cloudbuild BuildSteps for copying a repo and creating
// an output directory.
//
// The BuildStep for the repo copy uses id cloneStepID, as described in
// https://cloud.google.com/cloud-build/docs/build-config#id, for any future
// steps that need to depend on the repo clone step.  The repo copy step puts
// the code into /workspace/code.
//
// The output directory is /workspace/out.
func commonSteps(repo string) []*cloudbuild.BuildStep {
	return []*cloudbuild.BuildStep{
		&cloudbuild.BuildStep{
			Name:    constants.GCRGitImage, // This triggers with command 'git'.
			Args:    []string{"clone", repo, codeDirectory},
			Id:      cloneStepID,
			WaitFor: []string{"-"},
		},
		&cloudbuild.BuildStep{
			Name:    constants.GCRGitImage, // This triggers with command 'git'.
			Args:    []string{"checkout", defaultVersion},
			Dir:     codeDirectory,
			Id:      checkoutStepID,
			WaitFor: []string{cloneStepID},
		},
		&cloudbuild.BuildStep{
			Name:    "ubuntu", // This, however, has no entrypoint command.
			Args:    []string{"mkdir", "/workspace/out"},
			WaitFor: []string{"-"},
		},
	}
}

func preprocessorStep(build string, buildID int) *cloudbuild.BuildStep {
	return &cloudbuild.BuildStep{
		Name:    constants.KytheBuildPreprocessorImage,
		Args:    []string{build},
		Id:      preStepID + strconv.Itoa(buildID),
		WaitFor: []string{checkoutStepID},
	}
}

// TODO(#3095): This step needs to be configurable by the java version used for
// a given BuildTarget.
func javaExtractorsStep() *cloudbuild.BuildStep {
	return &cloudbuild.BuildStep{
		Name: constants.KytheJavacExtractorArtifactsImage,
		Volumes: []*cloudbuild.Volume{
			&cloudbuild.Volume{
				Name: javaVolumeName,
				Path: constants.DefaultExtractorsDir,
			},
		},
		Id: javaArtifactsID,
		// Don't need to wait for anything, because this is just sideloading
		// another docker image.
		WaitFor: []string{"-"},
	}
}

func zipMergeStep(corpus string) *cloudbuild.BuildStep {
	return &cloudbuild.BuildStep{
		Name:       constants.KytheKzipToolsImage,
		Entrypoint: "bash",
		Args: []string{
			"-c",
			fmt.Sprintf(
				"%s merge --output %s %s/*.kzip",
				constants.DefaultKzipToolLocation,
				path.Join(outputDirectory, outputFileName(corpus)),
				outputDirectory),
		},
		// We explicitly don't include a WaitFor here because that is equivalent
		// to waiting for everything.  This step should be done at the end, so
		// it gets no WaitFor (== wait for everything).
	}
}
