package server

import (
	"net/http"
	"path/filepath"
)

const (
	phabricatorURL = "https://phabricator-dot-kythe-repo.appspot.com"
	repoURL        = phabricatorURL + "/diffusion/K/browse/master"
	staticRoot     = "site"
)

func init() {
	http.Handle("/phabricator/", http.StripPrefix("/phabricator", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.Redirect(w, r, phabricatorURL+r.URL.Path, http.StatusTemporaryRedirect)
	})))
	http.Handle("/repo/", http.StripPrefix("/repo", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.Redirect(w, r, repoURL+r.URL.Path, http.StatusTemporaryRedirect)
	})))
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, filepath.Join(staticRoot, r.URL.Path))
	})
}
