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

	"kythe.io/kythe/go/languageserver/pathmap"
	"kythe.io/kythe/go/util/kytheuri"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

// mapping represents a single mapping to & from local paths and Kythe VName components
type mapping struct {
	local  *pathmap.Mapper
	corpus *pathmap.Mapper
	path   *pathmap.Mapper
	root   *pathmap.Mapper
}

// PathConfig provides the ability to map between paths locally and in Kythe
type PathConfig interface {
	// LocalFromURI generates a local file path from a DocumentURI usually
	// provided by the language client
	LocalFromURI(lspURI lsp.DocumentURI) (string, error)

	// KytheURIFromLocal generates a Kythe URI for the local file
	KytheURIFromLocal(loc string) (*kytheuri.URI, error)

	// LocalFromKytheURI generates an absolute local file path from a Kythe URI
	LocalFromKytheURI(ticket kytheuri.URI) (string, error)

	// Root returns the workspace root as determined by the PathConfig
	Root() string
}

// PathConfigProvider is used for delayed loading of path configs.
// It receives the editor suggested workspace root and construct a PathConfig.
type PathConfigProvider func(string) (PathConfig, error)

// SettingsPathConfig uses Settings values to map between paths locally and in Kythe
type SettingsPathConfig struct {
	// mappings is an ordered list of mapping options. The first one to match
	// will be used when converting to & from local paths and Kythe URIs
	mappings []*mapping

	// root is the local directory containing the code that is also indexed in
	// Kythe. All mappings will be relative to this location
	root string
}

// SettingsPathConfigProvider finds the setttings file and produces a SettingsPathConfig
func SettingsPathConfigProvider(possibleRoot string) (PathConfig, error) {
	s, err := findSettings(possibleRoot)
	if err != nil {
		return &SettingsPathConfig{}, err
	}
	return NewSettingsPathConfig(*s)
}

// NewSettingsPathConfig constructs a SettingsPathConfig from a settings object.
func NewSettingsPathConfig(s Settings) (*SettingsPathConfig, error) {
	p := SettingsPathConfig{}
	err := p.loadSettings(s)
	return &p, err
}

// Root returns the SettingsPathConfig's root
func (pc *SettingsPathConfig) Root() string {
	return pc.root
}

// loadSettings populates the SettingsPathConfig object with the
func (pc *SettingsPathConfig) loadSettings(s Settings) error {
	pc.root = s.Root
	for _, m := range s.Mappings {
		l, err := pathmap.NewMapper(m.Local)
		if err != nil {
			return err
		}

		c, err := pathmap.NewMapper(m.VName.Corpus)
		if err != nil {
			return err
		}

		p, err := pathmap.NewMapper(m.VName.Path)
		if err != nil {
			return err
		}

		r, err := pathmap.NewMapper(m.VName.Root)
		if err != nil {
			return err
		}

		pc.mappings = append(pc.mappings, &mapping{
			local:  l,
			corpus: c,
			path:   p,
			root:   r,
		})
	}
	return nil
}

// ticketFromRel attempts to apply a mapping to a local path relative to
// the root directory
func (m mapping) ticketFromRel(rel string) (*kytheuri.URI, error) {
	v, err := m.local.Parse(rel)
	if err != nil {
		return nil, err
	}

	c, err := m.corpus.Generate(v)
	if err != nil {
		return nil, err
	}

	p, err := m.path.Generate(v)
	if err != nil {
		return nil, err
	}

	r, err := m.root.Generate(v)
	if err != nil {
		return nil, err
	}

	return &kytheuri.URI{
		Corpus: c,
		Path:   p,
		Root:   r,
	}, nil
}

// relFromTicket attempts to apply a single mapping to a kythe ticket
// to produce a path relative to the root directory
func (m mapping) relFromTicket(ticket kytheuri.URI) (string, error) {
	cv, err := m.corpus.Parse(ticket.Corpus)
	if err != nil {
		return "", err
	}

	pv, err := m.path.Parse(ticket.Path)
	if err != nil {
		return "", err
	}

	rv, err := m.root.Parse(ticket.Root)
	if err != nil {
		return "", err
	}

	var vals = make(map[string]string)
	for k, v := range cv {
		vals[k] = v
	}
	for k, v := range pv {
		vals[k] = v
	}
	for k, v := range rv {
		vals[k] = v
	}

	return m.local.Generate(vals)
}

// LocalFromURI implements part of the PathConfig interface
func (pc *SettingsPathConfig) LocalFromURI(lspURI lsp.DocumentURI) (string, error) {
	u, err := url.Parse(string(lspURI))
	if err != nil {
		return "", err
	}

	if u.Scheme != "file" {
		return "", fmt.Errorf("only file:// uris can be handled. Given %s", lspURI)
	}

	if !strings.HasPrefix(u.Path, pc.root) {
		return "", fmt.Errorf("path '%s' does not start with root '%s'", u.Path, pc.root)
	}

	return u.Path, nil
}

// KytheURIFromLocal implements part of the PathConfig interface by
// iteratively attempting to apply mappings to the relative version of it
func (pc *SettingsPathConfig) KytheURIFromLocal(loc string) (*kytheuri.URI, error) {
	rel, err := filepath.Rel(pc.root, loc)
	if err != nil {
		return nil, fmt.Errorf("local file (%s) is not beneath root (%s)", loc, pc.root)
	}
	for _, m := range pc.mappings {
		t, err := m.ticketFromRel(rel)
		if err == nil {
			return t, nil
		}
	}

	return nil, fmt.Errorf("no matching config for local file (%s)", loc)

}

// LocalFromKytheURI implements part of the PathConfig interface by
// iteratively attempting to apply mappings to the given Kythe URI
func (pc *SettingsPathConfig) LocalFromKytheURI(ticket kytheuri.URI) (string, error) {
	for _, m := range pc.mappings {
		l, err := m.relFromTicket(ticket)
		if err == nil {
			return filepath.Join(pc.root, l), nil
		}
	}
	return "", fmt.Errorf("no matching config for ticket: %#v", ticket)
}
