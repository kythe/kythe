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
// Example:
//  kzip_validator -kzip <kzip-file> -local_repo <repo-root-dir> -lang cc,h
//  kzip_validator -kzip <kzip-file> -repo_url <repo-url> -lang java [-version <hash>]
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
	archiveSubdir = flag.String("archive_subdir", "REPO-VERSION", "This flag describes what the downloaded archive's format is.  Specify \"REPO-VESRION\" for a github-style nested subdirectory.  Specify \"\" emptystring for no nesting at all.")
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

func initFlags() {
	if len(flag.Args()) > 0 {
		log.Fatalf("Unknown arguments: %v", flag.Args())
	}

	if *kzip == "" {
		log.Fatal("You must provide at least one -kzip file")
	}
	if *repoURL == "" && *localRepo == "" {
		log.Fatal("You must specify either -repo_url or a -local_repo path")
	}
	if *repoURL != "" && *localRepo != "" {
		log.Fatal("You must specify only one of -repo_url or -local_repo")
	}
	if *lang == "" {
		log.Fatal("You must specify what -lang file extensions to examine")
	}
	if *repoURL != "" {
		if _, err := url.Parse(*repoURL); err != nil {
			log.Fatalf("Failed to parse -repo_url: %v", err)
		}
	}
	if *archiveSubdir != "REPO-VERSION" && *archiveSubdir != "" {
		log.Fatalf("Unsupported -archive_subdir %s, must use either \"REPO-VERSION\" or \"\"", *archiveSubdir)
	}
}

func main() {
	flag.Parse()
	initFlags()

	fmt.Printf("Validating kzip %s\n", *kzip)

	if err := validate(); err != nil {
		log.Fatalf("Error: %v", err)
	}
}

func validate() (retErr error) {
	repoPath, cleanup, err := fetchRepo()
	if err != nil {
		if cleanup != nil {
			if cerr := cleanup(); cerr != nil {
				fmt.Printf("Failure cleaning up: %v", cerr)
			}
		}
		return fmt.Errorf("failure fetching repo: %v", err)
	}
	defer func() {
		if cleanup != nil {
			if err := cleanup(); err != nil {
				log.Printf("Failed to clean up: %v", err)
				if retErr == nil {
					retErr = err
				}
			}
		}
	}()

	res, err := validation.Settings{
		Compilations:  strings.Split(*kzip, ","),
		Repo:          repoPath,
		Langs:         stringset.FromKeys(strings.Split(*lang, ",")),
		MissingOutput: missingFile,
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

// fetchRepo either just early returns an available -local_repo, or fetches it from
// the specified remote -repo_url
func fetchRepo() (string, func() error, error) {
	if *localRepo != "" {
		log.Printf("Comparing against local copy of repo %s", *localRepo)
		return *localRepo, nil, nil
	}
	if !supported(*archiveFormat) {
		return "", nil, fmt.Errorf("unsupported archive format: %s", *archiveFormat)
	}
	tmpdir, err := ioutil.TempDir("", "repo-dir")
	if err != nil {
		return "", nil, fmt.Errorf("creating tempdir: %v", err)
	}
	cleanup := func() error {
		return os.RemoveAll(tmpdir)
	}

	archive, err := fetchArchive(tmpdir)
	if err != nil {
		return "", cleanup, fmt.Errorf("fetching archive: %v", err)
	}

	if err := archiver.Unarchive(archive, tmpdir); err != nil {
		return "", cleanup, fmt.Errorf("extracting archive file: %v", err)
	}

	return relativedir(tmpdir), cleanup, nil
}

func supported(ext string) bool {
	_, err := archiver.ByExtension("foo" + ext)
	return err == nil
}

// fetchArchive downloads an archive specified by the format in url(), and puts
// it into a given temp directory.
func fetchArchive(dir string) (archive string, retErr error) {
	archivefile, err := os.Create(filepath.Join(dir, "repo-archive"+*archiveFormat))
	if err != nil {
		return "", fmt.Errorf("creating archive tmpfile: %v", err)
	}
	defer func() {
		if err := archivefile.Close(); err != nil {
			log.Printf("Failure closing tempfile: %v", err)
			if retErr == nil {
				retErr = err
			}
		}
	}()

	target := archiveurl()
	log.Printf("Downloading archive: %s", target)
	resp, err := http.Get(target)
	if err != nil {
		return "", fmt.Errorf("downloading archive: %v", err)
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
		return "", fmt.Errorf("copying archive: %v", err)
	}
	archive = archivefile.Name()
	return
}

func archiveurl() string {
	baseURL, err := url.Parse(*repoURL)
	if err != nil {
		log.Fatalf("Failed to parse -repo_url %s, but that should have been done at startup: %v", *repoURL, err)
	}
	baseURL.Path = path.Join(baseURL.Path, *archivePrefix, *version) + *archiveFormat
	return baseURL.String()
}

// Because some source repositories (for example github) provide archives that
// have a top-level directory in them, instead of just being flat repos, we
// may have to append a relative subdir.
func relativedir(dir string) string {
	switch *archiveSubdir {
	case "REPO-VERSION":
		return repoVersionRelative(dir)
	case "":
		return dir
	default:
		log.Fatalf("Invalid -archive_subdir %s, but that should have been found at startup", *archiveSubdir)
		return ""
	}
}

func repoVersionRelative(dir string) string {
	baseURL, err := url.Parse(*repoURL)
	if err != nil {
		log.Fatalf("Failed to parse -repo_url %s, but that should have been done at startup: %v", *repoURL, err)
	}
	// Get the last part, which we assume is the repo as in github.com/project/repo.
	parts := strings.Split(path.Clean(baseURL.Path), "/")
	if len(parts) == 0 {
		log.Fatal("Invalid -repo_url, but that should have been caught at startup")
	}
	return path.Join(dir, fmt.Sprintf("%s-%s", parts[len(parts)-1], *version))
}
