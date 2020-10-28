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

// Package modifier provides a library for adding a forked javac executable
// into a build.gradle file or pom.xml file.
package modifier // import "kythe.io/kythe/go/extractors/config/preprocessor/modifier"

import (
	"fmt"
	"io/ioutil"
	"os"
	"regexp"
)

// These are the lines necessary for gradle build to use a different javac.
const kytheJavacWrapper = `
allprojects {
  gradle.projectsEvaluated {
    tasks.withType(JavaCompile) {
      options.fork = true
      options.forkOptions.executable = '%s'
    }
  }
}
`

// This matches a line which sets the javac to use Kythe's javac-wrapper.sh
func kytheMatcher(javacWrapper string) *regexp.Regexp {
	return regexp.MustCompile(fmt.Sprintf(`\n\s*options\.forkOptions\.executable\ =\ '%s'`, javacWrapper))
}

// This matches any line which sets a new javac executable, useful for detecting
// edge cases which already modify javac.
var javacMatcher = regexp.MustCompile(`\n\s*options\.forkOptions\.executable\ =`)

// PreProcessBuildGradle takes a build.gradle file and either verifies that it
// already has the bits necessary to run kythe's javac wrapper, or adds that
// functionality.
//
// Note this potentially modifies the input file, so make a copy beforehand if
// you need to keep the original.
func PreProcessBuildGradle(buildGradleFile, javacWrapper string) error {
	k, err := hasKytheWrapper(buildGradleFile, javacWrapper)
	if err != nil {
		return err
	}
	if k {
		// Already has the kythe javac-wrapper.
		return nil
	}
	return appendKytheWrapper(buildGradleFile, javacWrapper)
}

func hasKytheWrapper(buildGradleFile, javacWrapper string) (bool, error) {
	bits, err := ioutil.ReadFile(buildGradleFile)
	if err != nil {
		return false, fmt.Errorf("reading file %s: %v", buildGradleFile, err)
	}
	if kytheMatcher(javacWrapper).Match(bits) {
		return true, nil
	}
	if javacMatcher.Match(bits) {
		return false, fmt.Errorf("found existing non-kythe javac override for file %s, which we can't handle yet", buildGradleFile)
	}
	return false, nil
}

func appendKytheWrapper(buildGradleFile, javacWrapper string) error {
	f, err := os.OpenFile(buildGradleFile, os.O_APPEND|os.O_WRONLY, os.ModeAppend)
	if err != nil {
		return fmt.Errorf("opening file %s for append: %v", buildGradleFile, err)
	}
	if _, err := fmt.Fprintf(f, kytheJavacWrapper, javacWrapper); err != nil {
		return fmt.Errorf("appending javac-wrapper to %s: %v", buildGradleFile, err)
	}
	return f.Close()
}
