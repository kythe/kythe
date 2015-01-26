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

// Binary server launches an HTTP server meant for appengine deployment. The
// server only handles three URLs:
//   /_ah/health  -- Returns an "ok" health check
//   /_ah/start   -- Does nothing
//   /watcher     -- Provides an GCS Object Change Notification endpoint
//
// The notifications are watched for schema documentation changes, and once a
// schema has been updated, the server calls "update_schema.sh
// gs://<bucket>/<schema_path>/*" with the expectations that it will begin
// serving that schema version ASAP.
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
	"time"

	"kythe/go/serving/xrefs"
	"kythe/go/serving/xrefs/tools"
	"kythe/go/serving/xrefs/web"
	"kythe/go/storage"
	"kythe/go/storage/gsutil"
)

var (
	listeningAddr    = flag.String("listen", "localhost:8081", "HTTP serving address")
	xrefsServingDir  = flag.String("xrefs_dir", "/srv/public", "Directory for XRefs Server static resources")
	schemaServingDir = flag.String("schema_dir", "/srv/schema", "Directory for schema static resources")

	updateDelay = flag.Duration("update_delay", 10*time.Second, "Delay to wait between notifications to update schema")
	bucketsFlag = flag.String("buckets", "kythe-builds", "Comma-separated list of valid buckets from which to download objects")

	initialStores = flag.String("initial_stores", "", `Comma-separated list of GCS URIs for GraphStores to initally serve (e.g. "gs://kythe-builds/graphstore_kythe_2014-10-15_14-42-11.tar.gz,gs://kythe-builds/graphstore_openjdk_2014-12-05_11-27-18.tar.gz")`)
)

type store struct {
	storage.GraphStore

	localPath string
	gcsURI    string
	timestamp time.Time
}

var (
	// schemaUpdates is a channel of update notifications known to be for the
	// Kythe schema documentation
	schemaUpdates = make(chan notification)
	// repoUpdates is a channel of update notifications known to be for a Kythe
	// graphstore
	repoUpdates = make(chan notification)

	// handlers is the currently serving xrefs.Service and FileTree
	handlers = &web.Handlers{}
	// graphstores is a map from name to a serving GraphStore
	graphstores = make(map[string]*store)

	// validBuckets is a set of bucket names that we handle in the GCS Object
	// Notification web hook
	validBuckets = make(map[string]bool)
)

func main() {
	flag.Parse()
	if *listeningAddr == "" {
		log.Fatal("Missing --listen address")
	} else if *bucketsFlag == "" {
		log.Fatal("Missing --buckets")
	} else if *initialStores == "" {
		log.Fatal("Missing --initial_stores")
	}

	for _, bucket := range strings.Split(*bucketsFlag, ",") {
		validBuckets[bucket] = true
	}

	// Ensure empty web.Handlers before --initial_stores are loaded
	if err := updateHandlers(); err != nil {
		log.Fatalf("Error initializing empty handlers: %v", err)
	}

	go func() {
		for _, uri := range strings.Split(*initialStores, ",") {
			repoUpdates <- fakeNotification(uri)
		}
	}()

	mux := http.NewServeMux()
	mux.HandleFunc("/_ah/health", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, "ok")
	})
	mux.HandleFunc("/_ah/start", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "sure, we'll start!")
	})
	mux.HandleFunc("/watcher", watcherHandler)
	web.AddXRefHandlers("/repo", handlers, mux, true)
	mux.Handle("/repo/", http.StripPrefix("/repo/", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, filepath.Join(*xrefsServingDir, filepath.Clean(r.URL.Path)))
	})))
	mux.Handle("/schema/", http.StripPrefix("/schema/", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		path := "schema.html"
		if r.URL.Path != "/" && r.URL.Path != "" {
			path = filepath.Clean(r.URL.Path)
		}
		http.ServeFile(w, r, filepath.Join(*schemaServingDir, path))
	})))
	mux.Handle("/", http.RedirectHandler("/schema/", 302))

	go schemaUpdater()
	go repoUpdater()

	log.Printf("Server listening on %q", *listeningAddr)
	log.Fatal(http.ListenAndServe(*listeningAddr, mux))
}

func updateHandlers() error {
	var stores []storage.GraphStore
	for _, gs := range graphstores {
		stores = append(stores, gs)
	}
	gs := storage.NewProxy(stores...)
	tree, err := tools.CreateFileTree(gs)
	if err != nil {
		return err
	}
	xs := xrefs.NewGraphStoreService(gs)

	handlers.XRefs = xs
	handlers.FileTree = tree
	return nil
}

// notification is a subset of a GCS Object Change Notification object
type notification struct {
	ChannelID   string
	ResourceURI string
	ResourceID  string

	Name    string
	Bucket  string
	Updated time.Time
}

func fakeNotification(uri string) notification {
	components := strings.Split(strings.TrimPrefix(uri, "gs://"), "/")
	return notification{
		Name:   filepath.Join(components[1:]...),
		Bucket: components[0],
	}
}

func schemaUpdater() {
	var (
		bucket        string
		latestVersion string
	)
	updateTimer := time.AfterFunc(*updateDelay, func() {
		if latestVersion == "" {
			return
		}

		log.Printf("Updating schema to version %q", latestVersion)
		files := fmt.Sprintf("gs://%s/%s/*", bucket, latestVersion)
		cmd := exec.Command("update_schema.sh", files)
		cmd.Stderr = os.Stderr
		if err := cmd.Run(); err != nil {
			log.Printf("Failed to update schema: %v", err)
		}
	})

	for n := range schemaUpdates {
		updateTimer.Reset(*updateDelay)
		bucket, latestVersion = n.Bucket, filepath.Dir(n.Name)
	}
}

func downloadGraphStore(key, uri string) (string, error) {
	dir, err := ioutil.TempDir("", "graphstore_"+key+"_")
	if err != nil {
		return "", fmt.Errorf("error creating directory: %v", err)
	}
	cmd := exec.Command("update_graphstore.sh", uri, dir)
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return "", fmt.Errorf("failed to download GraphStore at %q: %v", uri, err)
	}
	return dir, nil
}

var keyRE = regexp.MustCompile(`^gs://.*/graphstore(_(.+?))?_\d{4}-\d{2}-\d{2}_.*\.tar\.gz$`)

func updateGraphStore(uri string, timestamp time.Time, reload bool) error {
	m := keyRE.FindStringSubmatch(uri)
	if len(m) != 3 {
		return fmt.Errorf("invalid GraphStore URI %q", uri)
	}
	key := strings.TrimPrefix(m[2], "_")
	if key == "" {
		key = "default"
	}
	prevStore := graphstores[key]
	if prevStore != nil && !timestamp.After(prevStore.timestamp) {
		return fmt.Errorf("skipping %q GraphStore (already serving newer version %s)", key, prevStore.timestamp)
	}

	log.Printf("Downloading %v GraphStore (%s)", key, timestamp)
	path, err := downloadGraphStore(key, uri)
	if err != nil {
		return err
	}

	gs, err := gsutil.ParseGraphStore(path)
	if err != nil {
		return fmt.Errorf("error opening GraphStore: %v", err)
	} else if err := tools.AddReverseEdges(gs); err != nil {
		return fmt.Errorf("error processing GraphStore: %v", err)
	}
	graphstores[key] = &store{
		GraphStore: gs,
		localPath:  path,
		gcsURI:     uri,
		timestamp:  timestamp,
	}
	if reload {
		if err := updateHandlers(); err != nil {
			graphstores[key] = prevStore
			return fmt.Errorf("failed to update xref handlers: %v", err)
		}
	}

	if prevStore != nil {
		if err := os.RemoveAll(prevStore.localPath); err != nil {
			return fmt.Errorf("error removing previous GraphStore %q: %v", prevStore.localPath, err)
		}
	}
	return nil
}

func repoUpdater() {
	for n := range repoUpdates {
		uri := fmt.Sprintf("gs://%s/%s", n.Bucket, n.Name)
		if err := updateGraphStore(uri, n.Updated, true); err != nil {
			log.Println(err)
		}
	}
}

func watcherHandler(w http.ResponseWriter, r *http.Request) {
	state := firstOrEmpty(r.Header["X-Goog-Resource-State"])
	if state == "" {
		w.WriteHeader(http.StatusBadRequest)
		fmt.Fprintln(w, "Missing state header")
		return
	}

	d := json.NewDecoder(r.Body)
	var n notification
	if err := d.Decode(&n); err != nil {
		w.WriteHeader(http.StatusBadRequest)
		fmt.Fprintf(w, "Error decoding notification: %v\n", err)
		return
	}
	n.ChannelID = firstOrEmpty(r.Header["X-Goog-Channel-Id"])
	n.ResourceID = firstOrEmpty(r.Header["X-Goog-Resource-Id"])
	n.ResourceURI = firstOrEmpty(r.Header["X-Goog-Resource-Uri"])

	if n.ChannelID == "" || n.ResourceURI == "" || n.ResourceID == "" {
		w.WriteHeader(http.StatusBadRequest)
		fmt.Fprintf(w, "Invalid notification: %+v", n)
		return
	}

	log.Printf("Notification: %+v", n)
	if !validBuckets[n.Bucket] {
		log.Println("Skipping notification from unknown bucket")
		w.WriteHeader(http.StatusNoContent)
		return
	}

	switch state {
	case "sync":
	case "not_exists":
		w.WriteHeader(http.StatusNoContent)
	case "exists":
		switch {
		case strings.HasPrefix(n.Name, "schema_"):
			schemaUpdates <- n
		case strings.HasPrefix(n.Name, "graphstore_"):
			repoUpdates <- n
		default:
			log.Printf("Skipping unknown file: %q", n.Name)
		}
		w.WriteHeader(http.StatusNoContent)
	default:
		w.WriteHeader(http.StatusBadRequest)
		fmt.Fprintf(w, "Unknown notifcation state: %q\n", state[0])
	}
}

func firstOrEmpty(strs []string) string {
	if len(strs) == 0 {
		return ""
	}
	return strs[0]
}
