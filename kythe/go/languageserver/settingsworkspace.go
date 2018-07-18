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

package languageserver

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/url"
	"os"
	"path/filepath"
	"strings"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
	"kythe.io/kythe/go/languageserver/pathmap"
	"kythe.io/kythe/go/util/kytheuri"
)

const settingsFile = ".kythe_settings.json"

// VNameConfig contains pathmap patterns for VName components
type VNameConfig struct {
	Corpus string `json:"corpus"`
	Path   string `json:"path"`
	Root   string `json:"root"`
}

// MappingConfig contains pathmap patterns for local paths and VNames
type MappingConfig struct {
	Local string      `json:"local"`
	VName VNameConfig `json:"vname"`
}

// Settings contains the user configuration required for the server to
// communicate properly with its XRefClient
type Settings struct {
	Root     string          `json:"root"`
	Mappings []MappingConfig `json:"mappings"`
}

// SettingsWorkspace uses Settings values to map between paths locally and in Kythe
type SettingsWorkspace struct {
	// mappings is an ordered list of mapping options. The first one to match
	// will be used when converting to & from local paths and Kythe URIs
	mappings []*mapping

	// root is the local directory containing the code that is also indexed in
	// Kythe. All mappings will be relative to this location
	root string
}

// mapping represents a single mapping to & from local paths and Kythe VName components
type mapping struct {
	local  *pathmap.Mapper
	corpus *pathmap.Mapper
	path   *pathmap.Mapper
	root   *pathmap.Mapper
}

// NewSettingsWorkspaceFromURI finds the setttings file and produces a SettingsWorkspace
func NewSettingsWorkspaceFromURI(lspURI lsp.DocumentURI) (Workspace, error) {
	u, err := url.Parse(string(lspURI))
	if err != nil {
		return nil, err
	}

	s, err := findSettings(filepath.Dir(u.Path))
	if err != nil {
		return &SettingsWorkspace{}, err
	}
	return NewSettingsWorkspace(*s)
}

// NewSettingsWorkspace constructs a SettingsWorkspace from a settings object.
func NewSettingsWorkspace(s Settings) (*SettingsWorkspace, error) {
	p := SettingsWorkspace{}
	err := p.loadSettings(s)
	return &p, err
}

// Root returns the SettingsWorkspace's root
func (sw *SettingsWorkspace) Root() string {
	return sw.root
}

// loadSettings populates the SettingsWorkspace object with the
func (sw *SettingsWorkspace) loadSettings(s Settings) error {
	sw.root = s.Root
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

		sw.mappings = append(sw.mappings, &mapping{
			local:  l,
			corpus: c,
			path:   p,
			root:   r,
		})
	}
	return nil
}

// KytheURIFromRelative implements part of the Workspace interface by
// iteratively attempting to apply mappings to the relative version of it
func (sw *SettingsWorkspace) KytheURIFromRelative(rel string) (*kytheuri.URI, error) {
	for _, m := range sw.mappings {
		t, err := m.ticketFromRel(rel)
		if err == nil {
			return t, nil
		}
	}

	return nil, fmt.Errorf("no matching config for file %q in %q", rel, sw.root)

}

// LocalFromKytheURI implements part of the Workspace interface by
// iteratively attempting to apply mappings to the given Kythe URI
func (sw *SettingsWorkspace) LocalFromKytheURI(ticket kytheuri.URI) (LocalFile, error) {
	for _, m := range sw.mappings {
		rel, err := m.relFromTicket(ticket)
		if err == nil {
			return LocalFile{sw, rel}, nil
		}
	}
	return LocalFile{}, fmt.Errorf("no matching config for ticket: %#v", ticket)
}

// LocalFromURI implements part of the Workspace interface
func (sw *SettingsWorkspace) LocalFromURI(lspURI lsp.DocumentURI) (LocalFile, error) {
	u, err := url.Parse(string(lspURI))
	if err != nil {
		return LocalFile{}, err
	}

	if u.Scheme != "file" {
		return LocalFile{}, fmt.Errorf("only file:// uris can be handled. Given %s", lspURI)
	}

	if !strings.HasPrefix(u.Path, sw.root) {
		return LocalFile{}, fmt.Errorf("path '%s' does not start with root '%s'", u.Path, sw.root)
	}
	rel, err := filepath.Rel(sw.root, u.Path)
	if err != nil {
		return LocalFile{}, err
	}
	return LocalFile{sw, rel}, nil
}

// URIFromRelative implements part of the Workspace interface
func (sw *SettingsWorkspace) URIFromRelative(path string) string {
	return fmt.Sprintf("file://%s", filepath.Join(sw.root, path))
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

// findSettings searches upward from the given path until it finds
// a .kythe-settings.json file and attempts to read and parse that file
func findSettings(dir string) (*Settings, error) {
	root, err := FindEnclosing(dir, func(d string) bool {
		fi, err := os.Stat(filepath.Join(d, settingsFile))
		return err == nil && fi.Mode().IsRegular()
	})
	if err != nil {
		return nil, err
	}

	// Default settings
	var m []MappingConfig
	s := &Settings{
		Root:     root,
		Mappings: m,
	}

	dat, err := ioutil.ReadFile(filepath.Join(root, settingsFile))
	if err != nil {
		return s, err
	}

	if err := json.Unmarshal(dat, s); err != nil {
		return s, err
	}

	return s, nil
}
