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
	"path/filepath"

	"kythe.io/kythe/go/extractors/constants"
	rpb "kythe.io/kythe/proto/repo_go_proto"

	"google.golang.org/api/cloudbuild/v1"
)

type mavenGenerator struct{}

// preExtractSteps implements parts of buildSystemElaborator
func (m mavenGenerator) preExtractSteps() []*cloudbuild.BuildStep {
	return []*cloudbuild.BuildStep{
		javaExtractorsStep(),
	}
}

// extractSteps implements parts of buildSystemElaborator
func (m mavenGenerator) extractSteps(corpus string, target *rpb.ExtractionTarget, idSuffix string) []*cloudbuild.BuildStep {
	buildfile := path.Join(codeDirectory, target.Path)
	targetPath, _ := filepath.Split(target.Path)
	return []*cloudbuild.BuildStep{
		preprocessorStep(buildfile, idSuffix),
		&cloudbuild.BuildStep{
			Name:       constants.MvnJDK8Image,
			Entrypoint: "mvn",
			Args: []string{
				"clean",
				// TODO(#3126): If compile-test has to be done as a separate
				// step, then we also need to fix this in the same way as we do
				// for multiple repo support.  Probably this would need to be
				// done with multiple steps (but making sure to not clobber
				// output).
				// The alternative here is to fall back to using clean install,
				// which should also work.
				"compile",
				"test-compile",
				"-f", // Points directly at a specific pom.xml file:
				buildfile,
				"-Dmaven.compiler.forceJavacCompilerUse=true",
				"-Dmaven.compiler.fork=true",
				"-Dmaven.compiler.executable=" + constants.DefaultJavacWrapperLocation,
			},
			Volumes: []*cloudbuild.Volume{
				&cloudbuild.Volume{
					Name: javaVolumeName,
					Path: constants.DefaultExtractorsDir,
				},
			},
			Env: []string{
				"KYTHE_CORPUS=" + corpus,
				"KYTHE_OUTPUT_DIRECTORY=" + outputDirectory,
				"KYTHE_ROOT_DIRECTORY=" + filepath.Join(codeDirectory, targetPath),
				"JAVAC_EXTRACTOR_JAR=" + constants.DefaultJavaExtractorLocation,
				"REAL_JAVAC=" + constants.DefaultJavacLocation,
				"KYTHE_JAVA_RUNTIME_OPTIONS=-Xbootclasspath/p:" + constants.DefaultJava9ToolsLocation,
			},
			Id:      extractStepID + idSuffix,
			WaitFor: []string{javaArtifactsID, preStepID + idSuffix},
		},
	}
}

// postExtractSteps implements parts of buildSystemElaborator
func (m mavenGenerator) postExtractSteps(corpus string) []*cloudbuild.BuildStep {
	return []*cloudbuild.BuildStep{
		zipMergeStep(corpus),
	}
}

// defaultConfigFile implements parts of buildSystemElaborator
func (m mavenGenerator) defaultExtractionTarget() *rpb.ExtractionTarget {
	return &rpb.ExtractionTarget{
		Path: "pom.xml",
	}
}
