/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package analysis defines interfaces used to locate and analyze compilation
// units and their inputs.
//
// Implementations of the Fetcher interface express the ability to read file
// contents from index files, local files, index packs, and other storage.
package analysis

// A Fetcher provides the ability to fetch file contents from storage.
type Fetcher interface {
	// Fetch retrieves the contents of a single file.  At least one of path and
	// digest must be provided; both are preferred.  The implementation may
	// return an error if both are not set.
	Fetch(path, digest string) ([]byte, error)
}
