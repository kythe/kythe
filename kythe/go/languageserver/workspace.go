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
	"fmt"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/util/kytheuri"

	"github.com/sourcegraph/go-langserver/pkg/lsp"
)

// Workspace provides the ability to map between paths locally and in Kythe.
// Because Workspaces get copied into LocalFile objects, it is highly recommended
// that Workspace be implemented on pointer types
type Workspace interface {
	// LocalFromURI generates a local file path from a DocumentURI usually
	// provided by the language client
	LocalFromURI(lspURI lsp.DocumentURI) (LocalFile, error)

	// KytheURIFromRelative generates a Kythe URI for the local file
	KytheURIFromRelative(rel string) (*kytheuri.URI, error)

	// LocalFromKytheURI generates an absolute local file path from a Kythe URI
	LocalFromKytheURI(ticket kytheuri.URI) (LocalFile, error)

	// URIFromRelative generates a URI that can be passed to a language server
	URIFromRelative(rel string) string

	// Root returns the workspace root as determined by the Workspace
	Root() string
}

// LocalFile represents a file within a given workspace
type LocalFile struct {
	Workspace    Workspace // the workspace in which the file is found
	RelativePath string    // the path of the file within the workspace
}

func (l LocalFile) String() string {
	return filepath.Join(l.Workspace.Root(), l.RelativePath)
}

// KytheURI produces a kytheuri.URI object that represents the file remotely
func (l LocalFile) KytheURI() (*kytheuri.URI, error) {
	return l.Workspace.KytheURIFromRelative(l.RelativePath)
}

// URI produces a DocumentURI for the given LocalFile
func (l LocalFile) URI() lsp.DocumentURI {
	return lsp.DocumentURI(l.Workspace.URIFromRelative(l.RelativePath))
}

// FindEnclosing returns the nearest enclosing directory d of dir for which match(d)
// returns true. If no such directory is found, FindEnclosing returns os.ErrNotExist.
// Any other error is returned immediately.
func FindEnclosing(dir string, match func(string) bool) (string, error) {
	cur := filepath.Clean(dir)
	for {
		fs, err := os.Stat(cur)
		if err != nil {
			return "", err
		} else if !fs.IsDir() {
			return "", fmt.Errorf("not a directory: %q", cur)
		} else if match(cur) {
			return cur, nil
		} else if cur == "/" || cur == "." {
			break
		}
		cur = filepath.Dir(cur)
	}
	return "", os.ErrNotExist
}
