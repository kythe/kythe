/*
 * Copyright 2018 Google Inc. All rights reserved.
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

	"kythe.io/kythe/go/serving/pipeline/nodes"
	"kythe.io/kythe/go/util/kytheuri"

	"github.com/apache/beam/sdks/go/pkg/beam"

	srvpb "kythe.io/kythe/proto/serving_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

func init() {
	beam.RegisterFunction(addCorpusRootsKey)
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
		FilterByKind: []string{"file"},
		IncludeFacts: []string{},
		IncludeEdges: []string{},
	}, moveSourceToKey))
	return k.fileVNames
}

// CorpusRoots returns the single *srvpb.CorpusRoots key-value for the Kythe
// FileTree service.  The beam.PCollection has elements of type KV<string,
// *srvpb.CorpusRoots>.
func (k *KytheBeam) CorpusRoots() beam.PCollection {
	s := k.s.Scope("CorpusRoots")
	files := k.getFileVNames()
	return beam.ParDo(s, addCorpusRootsKey,
		beam.Combine(s, &combineCorpusRoots{}, beam.ParDo(s, fileToCorpusRoot, files)))
}

// Directories returns a Kythe *srvpb.FileDirectory table for the Kythe FileTree
// service.  The beam.PCollection has elements of type KV<string,
// *srvpb.FileDirectory>.
func (k *KytheBeam) Directories() beam.PCollection {
	s := k.s.Scope("Directories")
	files := k.getFileVNames()
	return beam.CombinePerKey(s, &combineDirectories{}, beam.ParDo(s, fileToDirectories, files))
}

// addCorpusRootsKey returns the given value with the Kythe corpus roots key constant.
func addCorpusRootsKey(val beam.T) (string, beam.T) { return "dirs:corpusRoots", val }

// fileToDirectories emits a FileDirectory for each path component in the given file VName.
func fileToDirectories(file *spb.VName, emit func(string, *srvpb.FileDirectory)) {
	// Clean the file path and remove any leading slash.
	path := filepath.Clean(filepath.Join("/", file.GetPath()))[1:]

	dir := &spb.VName{
		Corpus: file.Corpus,
		Root:   file.Root,
		Path:   currentAsEmpty(filepath.Dir(path)),
	}
	dirTicket := func() string { return fmt.Sprintf("dirs:%s\n%s\n%s", dir.Corpus, dir.Root, dir.Path) }
	emit(dirTicket(), &srvpb.FileDirectory{
		FileTicket: []string{kytheuri.ToString(file)},
	})
	for dir.Path != "" {
		ticket := kytheuri.ToString(dir)
		dir.Path = currentAsEmpty(filepath.Dir(dir.Path))
		emit(dirTicket(), &srvpb.FileDirectory{
			Subdirectory: []string{ticket},
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
	}
	return accum
}

func (combineCorpusRoots) ExtractOutput(cr *srvpb.CorpusRoots) *srvpb.CorpusRoots {
	sort.Slice(cr.Corpus, func(i, j int) bool { return cr.Corpus[i].Corpus < cr.Corpus[j].Corpus })
	for _, c := range cr.Corpus {
		c.Root = removeDuplicates(c.Root)
	}
	return cr
}

type combineDirectories struct{}

func (combineDirectories) MergeAccumulators(accum, dir *srvpb.FileDirectory) *srvpb.FileDirectory {
	accum.Subdirectory = append(accum.Subdirectory, dir.Subdirectory...)
	accum.FileTicket = append(accum.FileTicket, dir.FileTicket...)
	return accum
}

func (combineDirectories) ExtractOutput(dir *srvpb.FileDirectory) *srvpb.FileDirectory {
	dir.FileTicket = removeDuplicates(dir.FileTicket)
	dir.Subdirectory = removeDuplicates(dir.Subdirectory)
	return dir
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
