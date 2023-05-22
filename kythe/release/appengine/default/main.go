/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

package main

import (
	"fmt"
	"net/http"
	"os"
	"path/filepath"

	"kythe.io/kythe/go/util/log"
)

const (
	phabricatorURL = "https://phabricator-dot-kythe-repo.appspot.com"
	repoURL        = "https://github.com/kythe/kythe/tree/master"
	staticRoot     = "site"

	goGetHTML = `<html>
  <head>
    <meta charset="utf-8">
    <meta name="go-import" content="kythe.io git https://github.com/kythe/kythe">
    <meta name="go-source" content="kythe.io https://kythe.io https://kythe.io/repo{/dir} https://kythe.io/repo{/dir}/{file}${line}">
  </head>
  <body>
    <a href="https://kythe.io">kythe.io</a>
  </body>
</html>`
)

func main() {
	http.Handle("/phabricator/", http.StripPrefix("/phabricator", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.Redirect(w, r, phabricatorURL+r.URL.Path, http.StatusTemporaryRedirect)
	})))
	http.Handle("/repo/", http.StripPrefix("/repo", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.Redirect(w, r, repoURL+r.URL.Path, http.StatusTemporaryRedirect)
	})))
	// This redirect is put in place to handle the case when a user fails to include a trailing slash, which causes
	// images within docs/schema to not be properly served.
	http.Handle("/docs/schema", http.StripPrefix("/docs/schema", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.Redirect(w, r, "/docs/schema/", http.StatusTemporaryRedirect)
	})))
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		if _, ok := r.URL.Query()["go-get"]; ok {
			if _, err := w.Write([]byte(goGetHTML)); err != nil {
				log.Error("Error writing ?go-get=1 response:", err)
			}
		} else {
			http.ServeFile(w, r, filepath.Join(staticRoot, r.URL.Path))
		}
	})
	port := os.Getenv("PORT")
	if port == "" {
		port = "8080"
		log.Infof("Defaulting to port %s", port)
	}

	log.Infof("Listening on port %s", port)
	log.Fatal(http.ListenAndServe(fmt.Sprintf(":%s", port), nil))
}
