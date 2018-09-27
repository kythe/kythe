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
	requiredEnv = []string{"KYTHE_CORPUS", "KYTHE_ROOT_DIRECTORY", "KYTHE_OUTPUT_DIRECTORY"}
	// RequiredJavaEnv is all of the enivornment variables required for
	// extracting a java corpus, including env vars common for all extractors.
	RequiredJavaEnv = append(requiredEnv,
		// For example java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar
		"JAVAC_EXTRACTOR_JAR",
		// For example /usr/lib/jvm/java-8-openjdk/bin/javac
		"REAL_JAVAC",
		// If set to a file ending in '.kzip', this will cause the extractor to
		// output a .kzip file instead of multiple .kindex files.
		// Note this does not obviate the need to set KYTHE_OUTPUT_DIRECTORY.
		"KYTHE_OUTPUT_FILE",
	)
)
