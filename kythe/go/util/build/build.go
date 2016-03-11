/*
 * Copyright 2016 Google Inc. All rights reserved.
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

// Package build provides information about how a given binary was built and in
// what context.
package build

import "fmt"

var (
	_BUILD_SCM_REVISION    string
	_BUILD_SCM_STATUS      string
	_KYTHE_RELEASE_VERSION string
)

// VersionLine returns the following formatted string
// fmt.Sprintf("Version: %s [%s %s]", ReleaseVersion(), Status(), Revision()).
func VersionLine() string {
	return fmt.Sprintf("Version: %s [%s %s]", ReleaseVersion(), Status(), Revision())
}

// ReleaseVersion returns the Kythe release version for the current build.
func ReleaseVersion() string {
	if _KYTHE_RELEASE_VERSION != "" {
		return _KYTHE_RELEASE_VERSION
	}
	return Revision()
}

// Revision returns the source control revision for the current build.
func Revision() string {
	if _BUILD_SCM_REVISION != "" {
		return _BUILD_SCM_REVISION
	}
	return "HEAD"
}

// Status returns the source control status for the current build.
func Status() string {
	if _BUILD_SCM_STATUS != "" {
		return _BUILD_SCM_STATUS
	}
	return "Unknown"
}
