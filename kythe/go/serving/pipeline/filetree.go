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

package pipeline

import (
	"fmt"
	"path/filepath"
	"reflect"
	"sort"

	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
	"kythe.io/kythe/go/serving/pipeline/nodes"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/schema/facts"
	kinds "kythe.io/kythe/go/util/schema/nodes"

	"github.com/apache/beam/sdks/go/pkg/beam"

	scpb "kythe.io/kythe/proto/schema_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(addCorpusRootsKey)
	beam.RegisterFunction(anchorToBuildConfig)
	beam.RegisterFunction(anchorToCorpusRoot)
	beam.RegisterFunction(anchorToFileBuildConfig)
	beam.RegisterFunction(fileToCorpusRoot)
	beam.RegisterFunction(fileToDirectories)

	beam.RegisterType(reflect.TypeOf((*combineCorpusRoots)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*combineDirectories)(nil)).Elem())
}

func (k *KytheBeam) getFileVNames() beam.PCollection {
	if k.fileVNames.IsValid() {
		return k.fileVNames
	}
	k.fileVNames = beam.DropValue(k.s, beam.Seq(k.s, k.nodes, &nodes.Filter{
		FilterByKind: []string{kinds.File},
		IncludeFacts: []string{},
		IncludeEdges: []string{},
	}, moveSourceToKey))
	return k.fileVNames
}

func (k *KytheBeam) getAnchorBuildConfigs() beam.PCollection {
	if k.anchorBuildConfigs.IsValid() {
		return k.anchorBuildConfigs
	}
	k.anchorBuildConfigs = beam.Seq(k.s, k.nodes, &nodes.Filter{
		FilterByKind: []string{kinds.Anchor},
		IncludeFacts: []string{facts.BuildConfig},
		IncludeEdges: []string{},
	}, anchorToBuildConfig)
	return k.anchorBuildConfigs
}

func anchorToBuildConfig(anchor *scpb.Node) (*spb.VName, string) {
	var buildConfig string
	for _, f := range anchor.Fact {
		if f.GetKytheName() == scpb.FactName_BUILD_CONFIG {
			buildConfig = string(f.Value)
			break
		}
	}
	return anchor.Source, buildConfig
}

// CorpusRoots returns the single *srvpb.CorpusRoots key-value for the Kythe
// FileTree service.  The beam.PCollection has elements of type KV<string,
// *srvpb.CorpusRoots>.
func (k *KytheBeam) CorpusRoots() beam.PCollection {
	s := k.s.Scope("CorpusRoots")
	files := k.getFileVNames()
	anchors := k.getAnchorBuildConfigs()
	return beam.ParDo(s, addCorpusRootsKey,
		beam.Combine(s, &combineCorpusRoots{}, beam.Flatten(s,
			beam.ParDo(s, fileToCorpusRoot, files),
			beam.ParDo(s, anchorToCorpusRoot, anchors),
		)))
}

// Directories returns a Kythe *srvpb.FileDirectory table for the Kythe FileTree
// service.  The beam.PCollection has elements of type KV<string,
// *srvpb.FileDirectory>.
func (k *KytheBeam) Directories() beam.PCollection {
	s := k.s.Scope("Directories")
	files := k.getFileVNames()
	anchors := k.getAnchorBuildConfigs()
	return beam.CombinePerKey(s, &combineDirectories{}, beam.Flatten(s,
		beam.ParDo(s, fileToDirectories, files),
		beam.ParDo(s, anchorToFileBuildConfig, anchors),
	))
}

// addCorpusRootsKey returns the given value with the Kythe corpus roots key constant.
func addCorpusRootsKey(val beam.T) (string, beam.T) { return "dirs:corpusRoots", val }

func dirTicket(corpus, root, dir string) string {
	return fmt.Sprintf("dirs:%s\n%s\n%s", corpus, root, dir)
}

// anchorToFileBuildConfig emits a FileDirectory for each path component in the
// given anchor VName with its specified build config.
func anchorToFileBuildConfig(anchor *spb.VName, buildConfig string, emit func(string, *srvpb.FileDirectory)) {
	// Clean the file path and remove any leading slash.
	path := filepath.Clean(filepath.Join("/", anchor.GetPath()))[1:]
	dir := currentAsEmpty(filepath.Dir(path))
	buildConfigs := []string{buildConfig}

	corpus, root := anchor.GetCorpus(), anchor.GetRoot()
	emit(dirTicket(corpus, root, dir), &srvpb.FileDirectory{
		Entry: []*srvpb.FileDirectory_Entry{{
			Name:        filepath.Base(path),
			Kind:        srvpb.FileDirectory_FILE,
			BuildConfig: buildConfigs,
		}},
	})

	for dir != "" {
		name := filepath.Base(dir)
		dir = currentAsEmpty(filepath.Dir(dir))
		emit(dirTicket(corpus, root, dir), &srvpb.FileDirectory{
			Entry: []*srvpb.FileDirectory_Entry{{
				Name:        name,
				Kind:        srvpb.FileDirectory_DIRECTORY,
				BuildConfig: buildConfigs,
			}},
		})
	}
}

// fileToDirectories emits a FileDirectory for each path component in the given file VName.
func fileToDirectories(file *spb.VName, emit func(string, *srvpb.FileDirectory)) {
	// Clean the file path and remove any leading slash.
	path := filepath.Clean(filepath.Join("/", file.GetPath()))[1:]

	dir := currentAsEmpty(filepath.Dir(path))
	emit(dirTicket(file.Corpus, file.Root, dir), &srvpb.FileDirectory{
		Entry: []*srvpb.FileDirectory_Entry{{
			Name: filepath.Base(path),
			Kind: srvpb.FileDirectory_FILE,
		}},
	})
	for dir != "" {
		name := filepath.Base(dir)
		dir = currentAsEmpty(filepath.Dir(dir))
		emit(dirTicket(file.Corpus, file.Root, dir), &srvpb.FileDirectory{
			Entry: []*srvpb.FileDirectory_Entry{{
				Name: name,
				Kind: srvpb.FileDirectory_DIRECTORY,
			}},
		})
	}
}

func currentAsEmpty(p string) string {
	if p == "." {
		return ""
	}
	return p
}

// fileToCorpusRoot returns a CorpusRoots for the given file VName.
func fileToCorpusRoot(file *spb.VName) *srvpb.CorpusRoots {
	return &srvpb.CorpusRoots{
		Corpus: []*srvpb.CorpusRoots_Corpus{{
			Corpus: file.Corpus,
			Root:   []string{file.Root},
		}},
	}
}

// anchorToCorpusRoot returns a CorpusRoots for the anchor VName and build config.
func anchorToCorpusRoot(anchor *spb.VName, buildConfig string) *srvpb.CorpusRoots {
	return &srvpb.CorpusRoots{
		Corpus: []*srvpb.CorpusRoots_Corpus{{
			Corpus:      anchor.Corpus,
			Root:        []string{anchor.Root},
			BuildConfig: []string{buildConfig},
		}},
	}
}

type combineCorpusRoots struct{}

func (combineCorpusRoots) MergeAccumulators(accum, cr *srvpb.CorpusRoots) *srvpb.CorpusRoots {
	for _, c := range cr.Corpus {
		var corpus *srvpb.CorpusRoots_Corpus
		for _, cc := range accum.Corpus {
			if cc.Corpus == c.Corpus {
				corpus = cc
				break
			}
		}
		if corpus == nil {
			corpus = &srvpb.CorpusRoots_Corpus{Corpus: c.Corpus}
			accum.Corpus = append(accum.Corpus, corpus)
		}
		corpus.Root = append(corpus.Root, c.Root...)
		corpus.BuildConfig = append(corpus.BuildConfig, c.BuildConfig...)
	}
	return accum
}

func (combineCorpusRoots) ExtractOutput(cr *srvpb.CorpusRoots) *srvpb.CorpusRoots {
	sort.Slice(cr.Corpus, func(i, j int) bool { return cr.Corpus[i].Corpus < cr.Corpus[j].Corpus })
	for _, c := range cr.Corpus {
		c.Root = removeDuplicates(c.Root)
		c.BuildConfig = removeDuplicates(c.BuildConfig)
	}
	return cr
}

type combineDirectories struct{}

func (combineDirectories) MergeAccumulators(accum, dir *srvpb.FileDirectory) *srvpb.FileDirectory {
	accum.Entry = append(accum.Entry, dir.Entry...)
	return accum
}

func (combineDirectories) ExtractOutput(dir *srvpb.FileDirectory) *srvpb.FileDirectory {
	files := make(map[string]stringset.Set)
	subdirs := make(map[string]stringset.Set)
	for _, e := range dir.Entry {
		switch e.Kind {
		case srvpb.FileDirectory_FILE:
			if configs, ok := files[e.Name]; ok {
				configs.Add(e.BuildConfig...)
			} else {
				files[e.Name] = stringset.New(e.BuildConfig...)
			}
		case srvpb.FileDirectory_DIRECTORY:
			if configs, ok := subdirs[e.Name]; ok {
				configs.Add(e.BuildConfig...)
			} else {
				subdirs[e.Name] = stringset.New(e.BuildConfig...)
			}
		default:
			log.Warningf("unknown FileDirectory kind: %v", e.Kind)
		}
	}
	entries := make([]*srvpb.FileDirectory_Entry, 0, len(files)+len(subdirs))
	for file, configs := range files {
		entries = append(entries, &srvpb.FileDirectory_Entry{
			Kind:        srvpb.FileDirectory_FILE,
			Name:        file,
			BuildConfig: configs.Elements(),
		})
	}
	for subdir, configs := range subdirs {
		entries = append(entries, &srvpb.FileDirectory_Entry{
			Kind:        srvpb.FileDirectory_DIRECTORY,
			Name:        subdir,
			BuildConfig: configs.Elements(),
		})
	}
	sort.Slice(entries, func(i, j int) bool {
		return compare.Ints(int(entries[i].Kind), int(entries[j].Kind)).
			AndThen(entries[i].Name, entries[j].Name) == compare.LT
	})
	return &srvpb.FileDirectory{Entry: entries}
}

func removeDuplicates(strs []string) []string {
	if len(strs) <= 1 {
		return strs
	}
	sort.Strings(strs)
	j := 1
	for i := 1; i < len(strs); i++ {
		if strs[j-1] != strs[i] {
			strs[j] = strs[i]
			j++
		}
	}
	return strs[:j]
}
