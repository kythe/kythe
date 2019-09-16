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

	"kythe.io/kythe/go/platform/kzip"

	apb "kythe.io/kythe/proto/analysis_go_proto"
)

// KzipInfo scans a kzip and counts contained files and units, giving a breakdown by corpus and language.
func KzipInfo(f kzip.File) (*apb.KzipInfo, error) {
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
		corpusInfo(u.Proto.GetVName().GetCorpus()).Units[u.Proto.GetVName().GetLanguage()]++
		for _, ri := range u.Proto.RequiredInput {
			kzipInfo.TotalFiles++
			corpusInfo(ri.GetVName().GetCorpus()).Files[ri.GetVName().GetLanguage()]++
		}
		return nil
	})
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
