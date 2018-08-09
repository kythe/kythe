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

package mavencmd

import ()

// PreProcessPomXML takes a pom.xml file and either verifies that it already has
// the bits necessary to specify a separate compiler on commandline, or adds
// functionality by dropping in a maven-compiler-plugin to the build.
//
// Note this potentially modifies the input file, so make a copy beforehand if
// you need to keep the original.
func PreProcessPomXML(pomXMLFile string) error {
	k, err := hasCompilerPlugin(pomXMLFile)
	if err != nil {
		return err
	}
	if k {
		// Already has the kythe javac-wrapper.
		return nil
	}
	return nil
}

func hasCompilerPlugin(pomXMLFile string) (bool, error) {
	return false, nil
}

func appendCompilerPlugin(pomXMLFile string) error {
	return nil
}
