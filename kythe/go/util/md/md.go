/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

// Package md provides some helper functions for creating marked down for Kythe documentation.
package md

import (
	"fmt"
	"sort"
)

// LinkInfo contains information about the placement and destination of
// linkable text
type LinkInfo struct {
	StartByte   int
	Length      int
	Destination string
}

// Link creates a markdown link. Content must be html and md escaped prior to calling this function.
func Link(content, destination string) string {
	return fmt.Sprintf("[%s](%s)", content, destination)
}

// ProcessLinks adds markdown links to the provided content based on the
// provided link information. This content can be rendered by the client
func ProcessLinks(content string, links []LinkInfo) string {
	mdContent := ""
	currentPos := 0

	// Ensure links are in order of starting byte
	sort.Slice(links, func(i, j int) bool {
		return links[i].StartByte < links[j].StartByte
	})

	for _, linkInfo := range links {
		if linkInfo.StartByte >= len(content) {
			break
		}

		mdContent += content[currentPos:linkInfo.StartByte]
		currentPos = linkInfo.StartByte
		linkContent := content[currentPos : linkInfo.StartByte+linkInfo.Length]
		mdContent += Link(linkContent, linkInfo.Destination)
		currentPos += linkInfo.Length
	}

	if currentPos < len(content) {
		mdContent += content[currentPos:len(content)]
	}

	return mdContent
}
