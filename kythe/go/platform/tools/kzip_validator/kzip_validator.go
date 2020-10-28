/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

// Binary kzip_validator checks the contents of a kzip against a code repo and
// compares file coverage.
//
// Usage:
//  kzip_validator -kzip <kzip-file> -local_repo <repo-root-dir> -lang cc,h
//  kzip_validator -kzip <kzip-file> -repo_url <repo-url> -lang java [-version <hash>]
//
// Examples:
//  Github repos use default args (.zip, /archive/, master):
//    kzip_validator -kzip <kzip-file> -repo_url https://github.com/google/guava
//
//  Other repository hosting services may require additional arguments:
//    kzip_validator -kzip <kzip-file> \
//      -repo_url https://android.googlesource.com/project/superproject
//      -archive_prefix "+archive" \
//      -archive_format ".tar.gz" \
//      -archive_subdir "" \
//      -lang "cc,h"
//
package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"os"
	"path"
	"path/filepath"
	"strings"

	"kythe.io/kythe/go/extractors/validation"

	"bitbucket.org/creachadair/stringset"
	"github.com/mholt/archiver"
)

var (
	kzip      = flag.String("kzip", "", "The comma separated list of kzip files to check")
	repoURL   = flag.String("repo_url", "", "The repo to compare against")
	localRepo = flag.String("local_repo", "", "The path of an optional local repo to specify instead of -repo_url")
	lang      = flag.String("lang", "", "The comma-separated language file extensions to check, e.g. 'java' or 'cpp,h'")

	missingFile = flag.String("missing_file", "", "An optional file to write all missing filepaths to")

	// These flags use the url() function below to download the archive as:
	// <repo_url>/<archive_prefix>/<version><archive_format>
	//
	// For example: -repo_url http://github.com/google/guava
	// -> http://github.com/google/guava/archive/master.zip
	//
	// The supported archiveFormats are based on the dependent archiver lib:
	// https://github.com/mholt/archiver#supported-archive-formats
	version       = flag.String("version", "master", "The version of the remote repo to compare")
	archivePrefix = flag.String("archive_prefix", "archive", "The part of an archive download URL for a source repo, for example the 'archive' in https://github.com/google/guava/archive/version-hash.zip")
	archiveFormat = flag.String("archive_format", ".zip", "The file format of the downloaded archive")
	archiveSubdir = flag.String("archive_subdir", "REPO-VERSION", "This flag describes what the downloaded archive's format is.  Specify \"REPO-VERSION\" for a github-style nested subdirectory.  Specify \"\" emptystring for no nesting at all.")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s -kzip <kzip-file> -repo_url <url> [-version <hash>]

Compare a kzip file's contents with a given repo, and print results of file coverage.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

type repoConfig interface {
	// FetchRepo gets a repository and returns its path.
	FetchRepo() (string, error)
	// Cleanup removes any temporary files created during validation.
	Cleanup() error
}

func initFromFlags() (repoConfig, error) {
	initFail := false
	if len(flag.Args()) > 0 {
		log.Printf("Unknown arguments: %v", flag.Args())
		initFail = true
	}

	if *kzip == "" {
		log.Print("You must provide at least one -kzip file")
		initFail = true
	}
	if *repoURL == "" && *localRepo == "" {
		log.Print("You must specify either -repo_url or a -local_repo path")
		initFail = true
	}
	if *repoURL != "" && *localRepo != "" {
		log.Print("You must specify only one of -repo_url or -local_repo")
		initFail = true
	}
	if *lang == "" {
		log.Print("You must specify what -lang file extensions to examine")
		initFail = true
	}
	if *archiveSubdir != "REPO-VERSION" && *archiveSubdir != "" {
		log.Printf("Unsupported -archive_subdir %s, must use either \"REPO-VERSION\" or \"\"", *archiveSubdir)
		initFail = true
	}
	if !supported(*archiveFormat) {
		log.Printf("unsupported archive format: %s", *archiveFormat)
		initFail = true
	}

	if initFail {
		return nil, fmt.Errorf("invalid flags, aborting")
	}

	// Pick remote or local based on whether -repo_url or -local_repo is set.
	if *localRepo != "" {
		return &localRepoConfig{repo: *localRepo}, nil
	}
	return remoteConfigFromFlags()
}

type localRepoConfig struct {
	repo string
}

type remoteRepoConfig struct {
	remoteArchive    string
	tmpdir           string
	localArchivePath string
	localRepoPath    string
}

func remoteConfigFromFlags() (repoConfig, error) {
	baseURL, err := url.Parse(*repoURL)
	if err != nil {
		return nil, fmt.Errorf("failed to parse -repo_url: %v", err)
	}
	// We assume that the -repo_url uses github style url/project/repo format.
	dirs := strings.Split(path.Clean(baseURL.Path), "/")
	if len(dirs) == 0 {
		return nil, fmt.Errorf("-repo_url has no path: %s", *repoURL)
	}
	// repo could be "kythe" or "guava".
	repo := dirs[len(dirs)-1]

	tmpdir, err := ioutil.TempDir("", "repo-dir")
	if err != nil {
		return nil, fmt.Errorf("creating tempdir: %v", err)
	}
	baseURL.Path = path.Join(baseURL.Path, *archivePrefix, *version) + *archiveFormat

	config := &remoteRepoConfig{
		remoteArchive:    baseURL.String(),
		tmpdir:           tmpdir,
		localArchivePath: path.Join(tmpdir, "repo-archive"+*archiveFormat),
		localRepoPath:    tmpdir,
	}
	// Because some source repositories (for example github) provide archives
	// that have a top-level directory in them, instead of just being flat repos,
	// we append a relative subdir.
	if *archiveSubdir == "REPO-VERSION" {
		config.localRepoPath = path.Join(tmpdir, fmt.Sprintf("%s-%s", repo, *version))
	}
	return config, nil
}

func main() {
	flag.Parse()
	config, err := initFromFlags()
	if err != nil {
		log.Fatalf("%v", err)
	}

	fmt.Printf("Validating kzip %s\n", *kzip)

	if err := validate(config); err != nil {
		log.Fatalf("Error: %v", err)
	}
}

func validate(config repoConfig) (retErr error) {
	repoPath, err := config.FetchRepo()
	defer func() {
		if err := config.Cleanup(); err != nil {
			log.Printf("Failed to clean up: %v", err)
			if retErr == nil {
				retErr = err
			}
		}
	}()
	if err != nil {
		return fmt.Errorf("failure fetching repo: %v", err)
	}

	res, err := validation.Settings{
		Compilations:  strings.Split(*kzip, ","),
		Repo:          repoPath,
		Langs:         stringset.FromKeys(strings.Split(*lang, ",")),
		MissingOutput: *missingFile,
	}.Validate()
	if err != nil {
		return fmt.Errorf("failure validating: %v", err)
	}

	log.Printf("Result: %v", res)

	fmt.Println("KZIP verification:")
	fmt.Printf("KZIP file count: %d\n", res.NumArchiveFiles)
	fmt.Printf("Repo file count: %d\n", res.NumRepoFiles)
	fmt.Printf("Percent missing: %.1f%%\n", 100*float64(res.NumMissing)/float64(res.NumRepoFiles))
	if len(res.TopMissing.Paths) > 0 {
		fmt.Println("Top missing subdirectories:")
		for _, v := range res.TopMissing.Paths {
			fmt.Printf("%10d %s\n", v.Missing, v.Path)
		}
	}
	return
}

func (l localRepoConfig) FetchRepo() (string, error) {
	log.Printf("Comparing against local copy of repo %s", l.repo)
	return l.repo, nil
}

func (l localRepoConfig) Cleanup() error { return nil }

func (r remoteRepoConfig) FetchRepo() (string, error) {
	if err := r.fetchArchive(); err != nil {
		return "", fmt.Errorf("fetching archive: %v", err)
	}

	if err := archiver.Unarchive(r.localArchivePath, r.tmpdir); err != nil {
		return "", fmt.Errorf("extracting archive file: %v", err)
	}

	return r.localRepoPath, nil
}

func (r remoteRepoConfig) Cleanup() error {
	if r.tmpdir != "" {
		return os.RemoveAll(r.tmpdir)
	}
	return nil
}

func supported(ext string) bool {
	_, err := archiver.ByExtension("foo" + ext)
	return err == nil
}

// fetchArchive downloads an archive specified by the format in url(), and puts
// it into a given temp directory.
func (r remoteRepoConfig) fetchArchive() (retErr error) {
	archivefile, err := os.Create(r.localArchivePath)
	if err != nil {
		return fmt.Errorf("creating archive tmpfile: %v", err)
	}
	defer func() {
		if err := archivefile.Close(); err != nil {
			log.Printf("Failure closing tempfile: %v", err)
			if retErr == nil {
				retErr = err
			}
		}
	}()

	log.Printf("Downloading archive: %s", r.remoteArchive)
	resp, err := http.Get(r.remoteArchive)
	if err != nil {
		return fmt.Errorf("downloading archive: %v", err)
	}
	defer func() {
		if err := resp.Body.Close(); err != nil {
			log.Printf("Failure closing http body: %v", err)
			if retErr == nil {
				retErr = err
			}
		}
	}()

	if _, err = io.Copy(archivefile, resp.Body); err != nil {
		return fmt.Errorf("copying archive: %v", err)
	}
	return
}
