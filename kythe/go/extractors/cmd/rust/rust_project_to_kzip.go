/*
 * Copyright 2025 The Kythe Authors. All rights reserved.
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
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/platform/vfs"
	"kythe.io/kythe/go/util/vnameutil"
	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// the central state for the rust-project extractor
type extractor struct {
	project             rustProject
	projectRoot         string
	corpus              string
	rules               vnameutil.Rules
	kzipWriter          kzipWriterInterface
	requiredInputsCache *requiredInputsCache
}

// kzipWriterInterface defines the methods needed from a kzip writer.
type kzipWriterInterface interface {
	AddUnit(cu *apb.CompilationUnit, index *apb.IndexedCompilation_Index) (string, error)
	AddFile(r io.Reader) (string, error)
	Close() error // Add if Close is needed by the caller of writeCrate, though not writeCrate itself
}

// source is the set of directories which may have source files for this crate,
// as well as directories which are explicitly excluded from containing necessary source files.
type source struct {
	IncludeDirs []string `json:"include_dirs"`
	ExcludeDirs []string `json:"exclude_dirs"`
}

// dep is a dependency of a Crate
type dep struct {
	CrateId crateId `json:"crate"`
	Name    string  `json:"name"`
}

type crate struct {
	DisplayName       string   `json:"display_name,omitempty"`
	RootModule        string   `json:"root_module"`
	Edition           string   `json:"edition"`
	Deps              []dep    `json:"deps"`
	Cfg               []string `json:"cfg"`
	CrateId           crateId  `json:"-"` // do not output
	Label             string   `json:"label"`
	Target            string   `json:"target"`
	Source            source   `json:"source,omitempty"`
	IsWorkspaceMember bool     `json:"is_workspace_member"`
}

type crateId uint32

// A representation of rust-project.json.
// Note: doesn't include all fields which exist in rust-project.json, only the ones we need to convert to a kzip.
type rustProject struct {
	Crates []crate `json:"crates"`
}

// Go doesn't let you use a struct with a slice member as a key to a map
// So, define a struct which is of the type we want for a key, then give it a method to get a
// valid key to a map out of it.
type requiredInputMapKey string
type requiredInputCacheKey struct {
	includeDir   string
	exclude_dirs []string
}

func (k *requiredInputCacheKey) mapKey() requiredInputMapKey {
	sort.Slice(k.exclude_dirs, func(i, j int) bool {
		return k.exclude_dirs[i] < k.exclude_dirs[j]
	})
	return requiredInputMapKey(strings.Join(append([]string{k.includeDir}, k.exclude_dirs...), "|"))
}

type requiredInputValue struct {
	requiredInputs []*apb.CompilationUnit_FileInput
	sourceFiles    []string
}

// requiredInputsCache stores the results of file discovery operations to avoid
// redundant filesystem walks and kzip additions. It maps include directory paths
// (relative to the project root) to the files found within them. This is
// particularly useful when multiple crates share common dependency source directories.
type requiredInputsCache struct {
	cache map[requiredInputMapKey]*requiredInputValue
}

func newRequiredInputsCache() requiredInputsCache {
	return requiredInputsCache{
		cache: make(map[requiredInputMapKey]*requiredInputValue),
	}
}

// Check that the project root exists and is valid.
func ensureProjectRoot(projectRoot string) {
	if info, err := os.Stat(projectRoot); os.IsNotExist(err) {
		log.Fatalf("Project root does not exist: %s\n", projectRoot)
	} else if err == nil && !info.IsDir() {
		log.Fatalf("Project root is not a directory: %s\n", projectRoot)
	} else if err != nil {
		log.Fatalf("Error checking project root: %v\n", err)
	}
}

// Parse a RustProject from a path on the disk, then make all paths within that RustProject relative to a projectRoot.
func getProjectJSON(projectJSONPath string) rustProject {
	projectJsonFile, err := os.Open(projectJSONPath)
	if err != nil {
		log.Fatalf("Error opening rust-project.json: %v\n", err)
	}
	defer projectJsonFile.Close()

	var projectJson rustProject
	decoder := json.NewDecoder(projectJsonFile)
	err = decoder.Decode(&projectJson)
	if err != nil {
		log.Fatalf("Error decoding rust-project.json: %v\n", err)
	}

	return projectJson
}

// Make all paths in includeDirs, excludeDirs, and rootModule relative to the project root
func (project *rustProject) relativizeProjectJson(projectRoot string) {
	for i, crate := range project.Crates {
		for j, includeDir := range crate.Source.IncludeDirs {
			relativized, err := removeProjectRoot(includeDir, projectRoot)
			if err != nil {
				log.Printf("Error getting relative path between %s and %s, not relativizing:%v\n", includeDir, projectRoot, err)
				continue
			}

			project.Crates[i].Source.IncludeDirs[j] = relativized
		}

		for j, excludeDir := range crate.Source.ExcludeDirs {
			relativized, err := removeProjectRoot(excludeDir, projectRoot)
			if err != nil {
				log.Printf("Error getting relative path between %s and %s, not relativizing:%v\n", excludeDir, projectRoot, err)
				continue
			}
			project.Crates[i].Source.ExcludeDirs[j] = relativized
		}

		relativized, err := removeProjectRoot(crate.RootModule, projectRoot)
		if err != nil {
			log.Printf("Error getting relative path between %s and %s, not relativizing:%v\n", crate.RootModule, projectRoot, err)
			continue
		}
		project.Crates[i].RootModule = relativized

	}
}

// Remove the project root from a path, returning the relative path.
func removeProjectRoot(path string, projectRoot string) (string, error) {
	newPath, err := filepath.Rel(projectRoot, path)
	if err != nil {
		log.Printf("Error getting relative path between %s and %s, not relativizing:%v\n", path, projectRoot, err)
		return "", err
	}
	return newPath, nil
}

// Create a new kzip writer.
func newKzipWriter(ctx context.Context, outputZipPath string) *kzip.Writer {
	// Delete output zip if it already exists
	_ = os.RemoveAll(outputZipPath)

	out, err := vfs.Create(ctx, outputZipPath)
	if err != nil {
		log.Fatalf("Error creating output zip: %v\n", err)
	}

	kzipWriter, err := kzip.NewWriteCloser(out)
	if err != nil {
		log.Fatalf("Error creating kzip writer: %v\n", err)
	}
	return kzipWriter
}

// transitiveDepsForCrate performs a breadth-first search on the crate dependency graph
// starting from the given `crate`. It uses the direct dependency relationships defined
// in `crateDeps` to find all reachable crates.
func transitiveDepsForCrate(crate crate, crateDeps map[crateId][]crateId) []crateId {
	queue := []crateId{crate.CrateId}
	visited := make(map[crateId]bool)
	for len(queue) > 0 {
		currentCrateId := queue[0]
		queue = queue[1:] // Dequeue

		// Iterate over the direct dependencies of the current crate
		for _, dep := range crateDeps[currentCrateId] {
			// If the dependency hasn't been visited, add it to the queue and mark it as visited
			if _, ok := visited[dep]; !ok {
				queue = append(queue, dep)
				visited[dep] = true
			}
		}
	}
	transitiveDeps := make([]crateId, 0)
	for k := range visited {
		transitiveDeps = append(transitiveDeps, k)
	}
	sort.Slice(transitiveDeps, func(i, j int) bool {
		return transitiveDeps[i] < transitiveDeps[j]
	})

	return transitiveDeps
}

// getTransitiveDeps returns a map of all crate IDs to transitive crate dependencies, including itself
func getTransitiveDeps(crates []crate) [][]crateId {
	crateDeps := make(map[crateId][]crateId)
	for _, crate := range crates {
		// Add direct dependencies
		for _, dep := range crate.Deps {
			crateDeps[crate.CrateId] = append(crateDeps[crate.CrateId], dep.CrateId)
		}

		// Add the crate itself as its own dependency
		crateDeps[crate.CrateId] = append(crateDeps[crate.CrateId], crate.CrateId)
	}

	// get the transitive dependencies of each crate
	transitiveDeps := make([][]crateId, len(crates))
	for _, crate := range crates {
		transitiveDepsForCrate := transitiveDepsForCrate(crate, crateDeps)
		transitiveDeps[crate.CrateId] = transitiveDepsForCrate
	}
	return transitiveDeps
}

// getSourceDirs returns the set of included and excluded directories for all crates
// Additionally, we add the directory of the root module to the set of included directories.
// It returns a data structure containing the source directories and dependent directories, paired by include and exclude.
// Note: the returned source directories are NOT transitive
func getSourceDirs(crates []crate) map[crateId]source {
	perCrateSourceDirs := make(map[crateId]source)
	for _, crate := range crates {
		currentSourceDirs := source{
			IncludeDirs: []string{},
			ExcludeDirs: []string{},
		}
		currentSourceDirs.IncludeDirs = append(currentSourceDirs.IncludeDirs, crate.Source.IncludeDirs...)
		currentSourceDirs.ExcludeDirs = append(currentSourceDirs.ExcludeDirs, crate.Source.ExcludeDirs...)

		if len(crate.Source.IncludeDirs) == 0 {
			// Add the dir of the root module for the crate, which is implicitly included
			currentSourceDirs.IncludeDirs = append(currentSourceDirs.IncludeDirs, filepath.Dir(crate.RootModule))
		}

		perCrateSourceDirs[crate.CrateId] = currentSourceDirs
	}

	return perCrateSourceDirs
}

// whether we should include this file in the Compilation Unit based on the inclusions/exclusions in `sourceDirs`
func shouldIncludeFile(info os.FileInfo, path string, excludeDirs []string, projectRoot string) (bool, error) {
	if info.IsDir() {
		// check that this file isn't one of the dirs excluded by this crate
		for _, excludeDir := range excludeDirs {
			if filepath.Dir(path) == filepath.Join(projectRoot, excludeDir) {
				return false, filepath.SkipDir
			}
		}
		return false, nil
	}

	// only index .rs files for now. It's possible that some rust files depend on non rust files, like if they use include_bytes,
	// TODO(wittrock): figure out a better scheme for including all actually depended-on files and still exclude files like .deps and .rmeta
	if filepath.Ext(path) != ".rs" {
		return false, nil
	}

	return true, nil
}

type collectCrateSourcesFunc func(ctx context.Context, e *extractor, sourceDirs source) ([]string, []*apb.CompilationUnit_FileInput, error)

// collectCrateSourcesImpl walks the file system based on the provided sourceDirs,
// identifies relevant source files (currently only .rs files), adds them to the
// kzip archive, and returns lists of their relative paths and FileInput protos.
func collectCrateSourcesImpl(ctx context.Context, e *extractor, sourceDirs source) ([]string, []*apb.CompilationUnit_FileInput, error) {
	var sourceFiles []string
	var requiredInputs []*apb.CompilationUnit_FileInput

	// cache required inputs for each include_dir
	for _, includeDir := range sourceDirs.IncludeDirs {

		abspath := filepath.Join(e.projectRoot, includeDir)

		cacheKey := requiredInputCacheKey{
			includeDir:   abspath,
			exclude_dirs: sourceDirs.ExcludeDirs,
		}

		mapKey := cacheKey.mapKey()

		cacheVal := e.requiredInputsCache.cache[mapKey]
		if cacheVal != nil {
			requiredInputs = append(requiredInputs, cacheVal.requiredInputs...)
			sourceFiles = append(sourceFiles, cacheVal.sourceFiles...)

			// we don't need to add sources in this directory, because they've already been written if they're in the cache.
			continue
		}

		dirRequiredInputs := []*apb.CompilationUnit_FileInput{}
		err := vfs.Walk(ctx, abspath, func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}

			shouldInclude, err := shouldIncludeFile(info, path, sourceDirs.ExcludeDirs, e.projectRoot)
			if err != nil || !shouldInclude {
				return err
			}

			relativePath, err := removeProjectRoot(path, e.projectRoot)
			if err != nil {
				log.Fatalf("couldn't relativize %s to project root %s: %v", path, e.projectRoot, err)
			}

			input, err := vfs.Open(ctx, path)
			if err != nil {
				return err
			}
			defer input.Close()

			digest, err := e.kzipWriter.AddFile(input)
			if err != nil {
				return err
			}

			vname := &spb.VName{
				Corpus: e.corpus,
				Path:   relativePath,
			}

			if inferredVname, ok := e.rules.Apply(relativePath); ok {
				vname = inferredVname
				vname.Corpus = e.corpus
			}

			sourceFiles = append(sourceFiles, relativePath)
			dirRequiredInputs = append(dirRequiredInputs, &apb.CompilationUnit_FileInput{
				VName: vname,
				Info: &apb.FileInfo{
					Path:   relativePath,
					Digest: digest,
				},
			})
			return nil
		})

		// Some crates have include_dirs that don't actually exist on disk
		if err != nil && !os.IsNotExist(err) && err != filepath.SkipDir {
			log.Printf("not including %s: %v\n", abspath, err)
			continue
		}

		requiredInputs = append(requiredInputs, dirRequiredInputs...)

		e.requiredInputsCache.cache[mapKey] = &requiredInputValue{
			requiredInputs: dirRequiredInputs,
			sourceFiles:    sourceFiles,
		}
	}

	return sourceFiles, requiredInputs, nil
}

// Copies a crate, but does not clone strings
func deepCopyCrate(crate crate) crate {
	newCrate := crate
	newCrate.Deps = make([]dep, len(crate.Deps))
	copy(newCrate.Deps, crate.Deps)
	return newCrate
}

// synthesizeProjectJson creates a RustProject from a given crate and its transitive dependencies
func synthesizeProjectJson(id crateId, crates []crate, transitiveDeps [][]crateId) rustProject {
	project := rustProject{
		Crates: []crate{},
	}

	// Map of old crate ID -> new crate ID for all crates in this project
	newCrateIds := make(map[crateId]crateId)

	for _, dep := range transitiveDeps[id] {
		crate := deepCopyCrate(crates[dep])
		crate.IsWorkspaceMember = true
		newCrateIds[crate.CrateId] = crateId(len(project.Crates))
		project.Crates = append(project.Crates, crate)
	}

	// fix up crate deps - deps are defined by an index into the crates array,
	// but the new crates array no longer has the same crate ID space
	for i, crate := range project.Crates {
		for j, dep := range crate.Deps {
			project.Crates[i].Deps[j].CrateId = newCrateIds[dep.CrateId]
		}
	}

	return project
}

// return a rustProject as its JSON-encoded bytes
func getProjectBytes(project rustProject) *bytes.Reader {
	var buf bytes.Buffer
	err := json.NewEncoder(&buf).Encode(project)
	if err != nil {
		log.Fatalf("Error encoding project JSON: %v\n", err)
	}

	return bytes.NewReader(buf.Bytes())
}

// writeCrate processes a single Rust crate, determines its source files and all
// required inputs (including transitive dependencies), constructs a Kythe
// CompilationUnit protobuf, and adds the unit and its required files to the
// kzip archive. It utilizes a cache (`requiredInputsBuilder`) to efficiently find
// necessary files. The `rust-project.json` file is synthesized and added as a required
// input to every unit.
func (e *extractor) writeCrate(ctx context.Context, crate crate, transitiveDeps [][]crateId, sourceDirs map[crateId]source, collectCrateSources collectCrateSourcesFunc) error {
	// We need to write two sets of files to the kzip:
	// 1. the set of files in this crate, as Source files
	// 2. the other required inputs for this crate which aren't in the set of source files for this crate (dependencies)
	// We can use collectCrateSources for both, with a different argument of source dirs
	var crateFiles []string
	var err error
	if crateFiles, _, err = collectCrateSources(ctx, e, sourceDirs[crate.CrateId]); err != nil {
		log.Printf("Error getting source files for crate %s: %v\n", crate.Label, err)
		return err
	}

	// Get the set of depended-on files for the entire crate, not just direct source files
	var requiredInputs []*apb.CompilationUnit_FileInput = make([]*apb.CompilationUnit_FileInput, 0)
	for _, dep := range transitiveDeps[crate.CrateId] {
		_, depRequiredInputs, err := collectCrateSources(ctx, e, sourceDirs[dep])
		if err != nil {
			log.Printf("Error getting source files for crate %s: %v\n", crate.Label, err)
			return err
		}
		requiredInputs = append(requiredInputs, depRequiredInputs...)
	}

	// Synthesize a rust-project.json for this crate and add it to the CU
	projectJsonDigest, err := e.kzipWriter.AddFile(getProjectBytes(synthesizeProjectJson(crate.CrateId, e.project.Crates, transitiveDeps)))
	if err != nil {
		log.Printf("Error adding project json to kzip: %v\n", err)
		return err
	}
	projectJsonVname := &spb.VName{
		Corpus: e.corpus,
		Path:   "rust-project.json",
	}
	requiredInputs = append(requiredInputs, &apb.CompilationUnit_FileInput{
		VName: projectJsonVname,
		Info: &apb.FileInfo{
			Path:   "rust-project.json",
			Digest: projectJsonDigest,
		},
	})

	compilationUnit := &apb.CompilationUnit{
		VName: &spb.VName{
			Corpus:   e.corpus,
			Language: "rust",
		},
		RequiredInput: requiredInputs,
		SourceFile:    crateFiles,
	}

	digest, err := e.kzipWriter.AddUnit(compilationUnit, nil)
	if err != nil {
		log.Printf("Error adding compilation unit to kzip: %v, crate %s, digest: %s\n", err, crate.Label, digest)
		return err
	}

	return nil
}

func main() {
	projectJSONPath := flag.String("project_json", "", "Path to the rust-project.json file (required)")
	outputZipPath := flag.String("output", "", "Path for the output kzip file (required)")
	projectRoot := flag.String("root", "", "Directory to which all paths to source files are relative; the root directory to all possible source files. (required)")
	corpus := flag.String("corpus", "", "Corpus name for VNames (required)")
	vnamesJsonPath := flag.String("vnames_json_path", "", "Path to vnames.json (required)")
	crateFilter := flag.String("crate_filter", "", "optional, the module path for a specific crate to extract and output. All other crates will be ignored")

	flag.Parse()

	if *projectJSONPath == "" || *outputZipPath == "" || *projectRoot == "" || *corpus == "" || *vnamesJsonPath == "" {
		flag.Usage()
		os.Exit(1)
	}

	ctx := context.Background()

	ensureProjectRoot(*projectRoot)

	rules, err := vnameutil.LoadRules(*vnamesJsonPath)
	if err != nil {
		log.Fatalf("Error loading vname rules: %v", err)
	}

	projectJson := getProjectJSON(*projectJSONPath)
	projectJson.relativizeProjectJson(*projectRoot)

	// Assign crate IDs (dense)
	for id := range projectJson.Crates {
		projectJson.Crates[id].CrateId = crateId(id)
	}

	crates := projectJson.Crates

	// Many crates have the same source dirs they depend on.
	// Caching the map of crate -> files speeds this program up by ~two orders of magnitude for the Fuchsia corpus.
	requiredInputsCache := newRequiredInputsCache()

	// rust-project.json's deps aren't transitive, but Compilation Units deps are.
	// Get the set of source dirs we'll need files from, transitively.
	transitiveDeps := getTransitiveDeps(crates)
	sourceDirsPerCrate := getSourceDirs(crates)

	kzipWriter := newKzipWriter(ctx, *outputZipPath)

	extractor := extractor{
		project:             projectJson,
		projectRoot:         *projectRoot,
		corpus:              *corpus,
		rules:               rules,
		kzipWriter:          kzipWriter,
		requiredInputsCache: &requiredInputsCache,
	}

	cratesWritten := 0
	for _, crate := range crates {
		if *crateFilter != "" && *crateFilter != crate.RootModule {
			continue
		}

		if !crate.IsWorkspaceMember {
			// rust-project.json allows crates to be included in the file which aren't part of the workspace.
			// For completeness we choose to index them anyway.
			log.Printf("crate %s isn't a workspace member, but indexing anyway\n", crate.Label)
		}

		// If the root module for this crate doesn't exist on disk, skip the crate
		if _, err := os.Stat(filepath.Join(*projectRoot, crate.RootModule)); os.IsNotExist(err) {
			log.Printf("Root module does not exist: %s\n", crate.RootModule)
			continue
		}

		log.Printf("Adding crate %s...", crate.Label)
		err := extractor.writeCrate(ctx, crate, transitiveDeps, sourceDirsPerCrate, collectCrateSourcesImpl)
		fmt.Println(" done.")
		if err != nil {
			log.Printf("Error writing crate %s: %v\n", crate.Label, err)
			continue
		}
		cratesWritten++
	}

	log.Printf("Processed %d crates\n", len(crates))
	log.Printf("Wrote %d units\n", cratesWritten)
	kzipWriter.Close()
	log.Printf("Wrote zip to %s\n", *outputZipPath)
}
