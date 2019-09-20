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

// KzipInfo scans a kzip and counts contained files and units, giving a breakdown by corpus and language.
func KzipInfo(f kzip.File, scanOpts ...kzip.ScanOption) (*apb.KzipInfo, error) {
	// Get file and unit counts broken down by corpus, language.
	kzipInfo := &apb.KzipInfo{Corpora: make(map[string]*apb.KzipInfo_CorpusInfo)}
	corpusInfo := func(corpus string) *apb.KzipInfo_CorpusInfo {
		i := kzipInfo.Corpora[corpus]
		if i == nil {
			i = &apb.KzipInfo_CorpusInfo{
				Files: make(map[string]int32),
				Units: make(map[string]int32),
			}
			kzipInfo.Corpora[corpus] = i
		}
		return i
	}

	err := kzip.Scan(f, func(rd *kzip.Reader, u *kzip.Unit) error {
		kzipInfo.TotalUnits++

		// The corpus may be specified in the unit VName or in the source file
		// VNames. Record all values of corpus seen and afterwards check that a
		// single value is specified.
		var corpora stringset.Set
		if u.Proto.GetVName().GetCorpus() != "" {
			corpora.Add(u.Proto.GetVName().GetCorpus())
		}
		srcs := stringset.New(u.Proto.SourceFile...)

		for _, ri := range u.Proto.RequiredInput {
			kzipInfo.TotalFiles++
			// Determine language from the unit VName (note that file VNames are
			// forbidden from specifying a language).
			corpusInfo(ri.GetVName().GetCorpus()).Files[u.Proto.GetVName().GetLanguage()]++
			if srcs.Contains(ri.Info.Path) {
				corpora.Add(ri.GetVName().GetCorpus())
			}
		}
		if len(corpora) == 1 {
			corpusInfo(corpora.Elements()[0]).Units[u.Proto.GetVName().GetLanguage()]++
		} else {
			// This is a warning for now, but may become an error.
			log.Printf("Unable to determine unit corpus. unit vname={%v}; src corpora=%v; srcs=%v", u.Proto.GetVName(), corpora, u.Proto.SourceFile)
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
				Files: make(map[string]int32),
				Units: make(map[string]int32),
			}
			kzipInfo.Corpora[corpus] = i
		}
		return i
	}

	for _, i := range infos {
		kzipInfo.TotalUnits += i.TotalUnits
		kzipInfo.TotalFiles += i.TotalFiles
		for corpus, cinfo := range i.Corpora {
			mergedCorpInfo := corpusInfo(corpus)
			for lang, fileCount := range cinfo.Files {
				mergedCorpInfo.Files[lang] += fileCount
			}
			for lang, unitCount := range cinfo.Units {
				mergedCorpInfo.Units[lang] += unitCount
			}
		}
	}
	return kzipInfo
}
