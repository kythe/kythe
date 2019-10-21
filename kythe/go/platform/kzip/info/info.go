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

	err := kzip.Scan(f, func(rd *kzip.Reader, u *kzip.Unit) error {
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
		cuLang := u.Proto.GetVName().GetLanguage()
		cuInfo := cuLangInfo(cuLang, corpusInfo(cuCorpus, kzipInfo))
		cuInfo.Count++
		cuInfo.Sources += int32(srcs.Len())

		for _, ri := range u.Proto.RequiredInput {
			cuInfo.RequiredInputs++
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

// KzipInfoTotalCount returns the total CompilationUnits counts for infos split apart by language.
func KzipInfoTotalCount(infos []*apb.KzipInfo) apb.KzipInfo_CorpusInfo {
	totals := apb.KzipInfo_CorpusInfo{
		LanguageCompilationUnits: make(map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits),
	}
	for _, info := range infos {
		for _, i := range info.GetCorpora() {
			for lang, stats := range i.GetLanguageCompilationUnits() {
				langTotal := totals.LanguageCompilationUnits[lang]
				if langTotal == nil {
					langTotal = &apb.KzipInfo_CorpusInfo_CompilationUnits{}
					totals.LanguageCompilationUnits[lang] = langTotal
				}
				langTotal.Count += stats.GetCount()
				langTotal.Sources += stats.GetSources()
				langTotal.RequiredInputs += stats.GetRequiredInputs()
			}
		}
	}
	return totals
}

// MergeKzipInfo combines the counts from multiple KzipInfos.
func MergeKzipInfo(infos []*apb.KzipInfo) *apb.KzipInfo {
	kzipInfo := &apb.KzipInfo{Corpora: make(map[string]*apb.KzipInfo_CorpusInfo)}

	for _, i := range infos {
		for corpus, cinfo := range i.GetCorpora() {
			mergedCorpInfo := corpusInfo(corpus, kzipInfo)
			for lang, cu := range cinfo.GetLanguageCompilationUnits() {
				cui := cuLangInfo(lang, mergedCorpInfo)
				cui.Count += cu.GetCount()
				cui.Sources += cu.GetSources()
				cui.RequiredInputs += cu.GetRequiredInputs()
			}
		}
	}
	return kzipInfo
}

func corpusInfo(corpus string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo {
	i := kzipInfo.GetCorpora()[corpus]
	if i == nil {
		i = &apb.KzipInfo_CorpusInfo{
			LanguageCompilationUnits: make(map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits),
		}
		kzipInfo.Corpora[corpus] = i
	}
	return i
}

func cuLangInfo(lang string, c *apb.KzipInfo_CorpusInfo) *apb.KzipInfo_CorpusInfo_CompilationUnits {
	cui := c.LanguageCompilationUnits[lang]
	if cui == nil {
		cui = &apb.KzipInfo_CorpusInfo_CompilationUnits{}
		c.LanguageCompilationUnits[lang] = cui
	}
	return cui
}
