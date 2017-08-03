/*
 * Copyright 2017 Google Inc. All rights reserved.
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

package languageserver

import (
	"fmt"
	"net/url"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/util/kytheuri"
)

// PathConfig provides the ability to map between paths locally and in Kythe
type PathConfig struct {
	Root   string
	Corpus string
}

// localFromURI generates a path relative to the root from a file URI
func (p *PathConfig) localFromURI(lspURI string) (string, error) {
	// TODO: Change this parameter to lsp.DocumentURI.
	u, err := url.Parse(lspURI)
	if err != nil {
		return "", err
	}

	if u.Scheme != "file" {
		return "", fmt.Errorf("Only file:// uris can be handled. Given %s", lspURI)
	}

	if !strings.HasPrefix(u.Path, p.Root) {
		return "", fmt.Errorf("Path '%s' does not start with root '%s'", u.Path, p.Root)
	}

	return u.Path, nil
}

func (p *PathConfig) ticketFromLocal(loc string) (*kytheuri.URI, error) {
	trunc, err := filepath.Rel(p.Root, loc)
	if err != nil {
		return nil, fmt.Errorf("Error truncating path (%s) with root (%s)", loc, p.Root)
	}
	return &kytheuri.URI{
		Corpus: p.Corpus,
		Path:   trunc,
	}, nil
}

func (p *PathConfig) localFromTicket(ticket kytheuri.URI) (string, error) {
	return filepath.Join(p.Root, ticket.Path), nil
}
