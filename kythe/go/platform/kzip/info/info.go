/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Package info provides utilities for summarizing the contents of a kzip.
package info // import "kythe.io/kythe/go/platform/kzip/info"

import (
	"fmt"
	"log"

	"bitbucket.org/creachadair/stringset"

	"kythe.io/kythe/go/platform/kzip"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

// If the compilation unit doesn't set a corpus, use this corpus so we have somewhere to record the
// stats.
const unknownCorpus = "__UNKNOWN_CORPUS__"

// KzipInfo scans a kzip and counts contained files and units, giving a breakdown by corpus and language.
func KzipInfo(f kzip.File, scanOpts ...kzip.ScanOption) (*apb.KzipInfo, error) {
	// Get file and unit counts broken down by corpus, language.
	kzipInfo := &apb.KzipInfo{Corpora: make(map[string]*apb.KzipInfo_CorpusInfo)}
	corpusInfo := func(corpus string) *apb.KzipInfo_CorpusInfo {
		i := kzipInfo.Corpora[corpus]
		if i == nil {
			i = &apb.KzipInfo_CorpusInfo{
				Units:              make(map[string]int32),
				SourceFiles:        make(map[string]int32),
				RequiredInputFiles: make(map[string]int32),
			}
			kzipInfo.Corpora[corpus] = i
		}
		return i
	}

	err := kzip.Scan(f, func(rd *kzip.Reader, u *kzip.Unit) error {
		kzipInfo.TotalUnits++

		srcs := stringset.New(u.Proto.SourceFile...)
		// The corpus may be specified in the unit VName or in the source file
		// VNames. Record all values of corpus seen and afterwards check that a
		// single value is specified.
		var corpora stringset.Set
		cuCorpus := u.Proto.GetVName().GetCorpus()
		if cuCorpus != "" {
			corpora.Add(cuCorpus)
		} else {
			log.Printf("Warning: Corpus not set for compilation unit %v", u.Proto.GetVName())
			cuCorpus = unknownCorpus
		}
		corpusInfo(cuCorpus).SourceFiles[u.Proto.GetVName().GetLanguage()] += int32(srcs.Len())
		kzipInfo.TotalSourceFiles += int32(srcs.Len())
		corpusInfo(cuCorpus).Units[u.Proto.GetVName().GetLanguage()]++

		for _, ri := range u.Proto.RequiredInput {
			kzipInfo.TotalRequiredInputFiles++
			// Determine language from the unit VName (note that file VNames are
			// forbidden from specifying a language).
			corpusInfo(ri.GetVName().GetCorpus()).RequiredInputFiles[u.Proto.GetVName().GetLanguage()]++
			if srcs.Contains(ri.Info.Path) {
				corpora.Add(ri.GetVName().GetCorpus())
			}
		}
		if len(corpora) != 1 {
			// This is a warning for now, but may become an error.
			log.Printf("Multiple corpora in unit. unit vname={%v}; src corpora=%v; srcs=%v", u.Proto.GetVName(), corpora, u.Proto.SourceFile)
		}
		return nil
	}, scanOpts...)
	if err != nil {
		return nil, fmt.Errorf("scanning kzip: %v", err)
	}

	return kzipInfo, nil
}

// MergeKzipInfo combines the counts from multiple KzipInfos.
func MergeKzipInfo(infos []*apb.KzipInfo) *apb.KzipInfo {
	kzipInfo := &apb.KzipInfo{Corpora: make(map[string]*apb.KzipInfo_CorpusInfo)}
	corpusInfo := func(corpus string) *apb.KzipInfo_CorpusInfo {
		i := kzipInfo.Corpora[corpus]
		if i == nil {
			i = &apb.KzipInfo_CorpusInfo{
				Units:              make(map[string]int32),
				SourceFiles:        make(map[string]int32),
				RequiredInputFiles: make(map[string]int32),
			}
			kzipInfo.Corpora[corpus] = i
		}
		return i
	}

	for _, i := range infos {
		kzipInfo.TotalUnits += i.TotalUnits
		kzipInfo.TotalRequiredInputFiles += i.TotalRequiredInputFiles
		kzipInfo.TotalSourceFiles += i.TotalSourceFiles
		for corpus, cinfo := range i.Corpora {
			mergedCorpInfo := corpusInfo(corpus)
			for lang, fileCount := range cinfo.RequiredInputFiles {
				mergedCorpInfo.RequiredInputFiles[lang] += fileCount
			}
			for lang, fileCount := range cinfo.SourceFiles {
				mergedCorpInfo.SourceFiles[lang] += fileCount
			}
			for lang, unitCount := range cinfo.Units {
				mergedCorpInfo.Units[lang] += unitCount
			}
		}
	}
	return kzipInfo
}
