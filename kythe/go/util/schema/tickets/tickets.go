/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

// Package tickets defines functions for manipulating Kythe tickets.
package tickets // import "kythe.io/kythe/go/util/schema/tickets"

import "kythe.io/kythe/go/util/kytheuri"

// AnchorFile constructs the canonical file ticket corresponding to a given
// anchor ticket. It is an error only if anchofTicket is invalid.
func AnchorFile(anchorTicket string) (string, error) {
	// Use ParseRaw to avoid the expense of unescaping, which we don't need.
	r, err := kytheuri.ParseRaw(anchorTicket)
	if err != nil {
		return "", err
	}
	// See http://www.kythe.io/docs/schema/#anchor for vname rules.
	r.URI.Signature = ""
	r.URI.Language = ""
	return r.String(), nil
}
