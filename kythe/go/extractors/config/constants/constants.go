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

// Package constants defines relevant constants common to multiple extractors.
package constants

var (
	// TODO(#3151): KYTHE_CORPUS probably should not be set via env var.
	requiredEnv = []string{"KYTHE_CORPUS", "KYTHE_ROOT_DIRECTORY", "KYTHE_OUTPUT_DIRECTORY"}
	// RequiredJavaEnv is all of the enivornment variables required for
	// extracting a java corpus, including env vars common for all extractors.
	RequiredJavaEnv = append(requiredEnv,
		// For example java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar
		"JAVAC_EXTRACTOR_JAR",
		// For example /usr/lib/jvm/java-8-openjdk/bin/javac
		"REAL_JAVAC",
		// For example with java8, this would be
		// -Xbootclasspath/p:/opt/kythe/extractors/javac9_tools.jar
		"KYTHE_JAVA_RUNTIME_OPTIONS",
		// If set to a file ending in '.kzip', this will cause the extractor to
		// output a .kzip file instead of multiple .kindex files.
		// Note this does not obviate the need to set KYTHE_OUTPUT_DIRECTORY.
		"KYTHE_OUTPUT_FILE",
	)

	// DefaultJavacLocation points to a common location for a javac binary.
	// The binary will usually be symlinked here.
	DefaultJavacLocation = "/usr/bin/javac"

	// These are specified in the image
	// gcr.io/kythe-public/kythe-javac-extractor-artifacts

	// DefaultJavacWrapperLocation is the location of the Kythe wrapper around
	// javac that does extraction.
	DefaultJavacWrapperLocation = "/opt/kythe/extractors/javac-wrapper.sh"
	// DefaultJavaExtractorLocation is the location of the actual extractor.
	DefaultJavaExtractorLocation = "/opt/kythe/extractors/javac_extractor.jar"
	// DefaultJava9ToolsLocation is the location of a jar which allows java9
	// compatibility for java8.
	DefaultJava9ToolsLocation = "/opt/kythe/extractors/java9_tools.jar"
)
