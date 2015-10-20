/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package server

import (
	"log"
	"net/http"
	"path/filepath"
)

const (
	phabricatorURL = "https://phabricator-dot-kythe-repo.appspot.com"
	repoURL        = phabricatorURL + "/diffusion/K/browse/master"
	staticRoot     = "site"

	goGetHTML = `<html>
  <head>
    <meta charset="utf-8">
    <meta name="go-import" content="kythe.io git https://github.com/google/kythe">
    <meta name="go-source" content="kythe.io https://kythe.io https://kythe.io/repo{/dir} https://kythe.io/repo{/dir}/{file}${line}">
  </head>
  <body>
    <a href="https://kythe.io">kythe.io</a>
  </body>
</html>`
)

func init() {
	http.Handle("/phabricator/", http.StripPrefix("/phabricator", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.Redirect(w, r, phabricatorURL+r.URL.Path, http.StatusTemporaryRedirect)
	})))
	http.Handle("/repo/", http.StripPrefix("/repo", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.Redirect(w, r, repoURL+r.URL.Path, http.StatusTemporaryRedirect)
	})))
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		if _, ok := r.URL.Query()["go-get"]; ok {
			if _, err := w.Write([]byte(goGetHTML)); err != nil {
				log.Println("Error writing ?go-get=1 response:", err)
			}
		} else {
			http.ServeFile(w, r, filepath.Join(staticRoot, r.URL.Path))
		}
	})
}
