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
const unspecifiedCorpus = "__UNSPECIFIED_CORPUS__"

// KzipInfo scans a kzip and counts contained files and units, giving a breakdown by corpus and language.
func KzipInfo(f kzip.File, scanOpts ...kzip.ScanOption) (*apb.KzipInfo, error) {
	// Get file and unit counts broken down by corpus, language.
	kzipInfo := &apb.KzipInfo{Corpora: make(map[string]*apb.KzipInfo_CorpusInfo)}

	err := kzip.Scan(f, func(rd *kzip.Reader, u *kzip.Unit) error {
		srcs := stringset.New(u.Proto.SourceFile...)
		// The corpus may be specified in the unit VName or in the source file
		// VNames. Record all values of corpus seen and afterwards check that a
		// single value is specified.
		cuCorpus := u.Proto.GetVName().GetCorpus()
		if cuCorpus == "" {
			log.Printf("Warning: Corpus not set for compilation unit %v", u.Proto.GetVName())
			cuCorpus = unspecifiedCorpus
		}
		cuLang := u.Proto.GetVName().GetLanguage()
		cuInfo := cuLangInfo(cuCorpus, cuLang, kzipInfo)
		cuInfo.Count++

		var srcCorpora stringset.Set
		for _, ri := range u.Proto.RequiredInput {
			riCorpus := requiredInputCorpus(u, ri)
			requiredInputInfo(riCorpus, cuLang, kzipInfo).Count++
			if srcs.Contains(ri.Info.Path) {
				sourceInfo(riCorpus, cuLang, kzipInfo).Count++
				srcCorpora.Add(riCorpus)
			}
		}
		if srcCorpora.Len() != 1 {
			// This is a warning for now, but may become an error.
			log.Printf("Multiple corpora in unit. unit vname={%v}; src corpora=%v; srcs=%v", u.Proto.GetVName(), srcCorpora, u.Proto.SourceFile)
		}
		return nil
	}, scanOpts...)
	if err != nil {
		return nil, fmt.Errorf("scanning kzip: %v", err)
	}

	return kzipInfo, nil
}

// requiredInputCorpus computes the corpus for a required input. It follows the rules in the
// CompilationUnit proto comments in kythe/proto/analysis.proto that say that any
// required_input that does not set corpus in its VName should inherit corpus from the compilation
// unit's VName.
func requiredInputCorpus(u *kzip.Unit, ri *apb.CompilationUnit_FileInput) string {
	if c := ri.GetVName().GetCorpus(); c != "" {
		return c
	}
	return u.Proto.GetVName().GetCorpus()
}

// KzipInfoTotalCount returns the total CompilationUnits counts for infos split apart by language.
func KzipInfoTotalCount(infos []*apb.KzipInfo) apb.KzipInfo_CorpusInfo {
	totals := apb.KzipInfo_CorpusInfo{
		LanguageCompilationUnits: make(map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits),
		LanguageRequiredInputs:   make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
		LanguageSources:          make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
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
			}
			for lang, stats := range i.GetLanguageRequiredInputs() {
				total := totals.LanguageRequiredInputs[lang]
				if total == nil {
					total = &apb.KzipInfo_CorpusInfo_RequiredInputs{}
					totals.LanguageRequiredInputs[lang] = total
				}
				total.Count += stats.GetCount()
			}
			for lang, stats := range i.GetLanguageSources() {
				total := totals.LanguageSources[lang]
				if total == nil {
					total = &apb.KzipInfo_CorpusInfo_RequiredInputs{}
					totals.LanguageSources[lang] = total
				}
				total.Count += stats.GetCount()
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
			for lang, cu := range cinfo.GetLanguageCompilationUnits() {
				cui := cuLangInfo(corpus, lang, kzipInfo)
				cui.Count += cu.GetCount()
			}
			for lang, inputs := range cinfo.GetLanguageRequiredInputs() {
				c := requiredInputInfo(corpus, lang, kzipInfo)
				c.Count += inputs.GetCount()
			}
			for lang, sources := range cinfo.GetLanguageSources() {
				c := sourceInfo(corpus, lang, kzipInfo)
				c.Count += sources.GetCount()
			}
		}
	}
	return kzipInfo
}

func cuLangInfo(corpus, lang string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo_CompilationUnits {
	c := corpusInfo(corpus, kzipInfo)
	cui := c.LanguageCompilationUnits[lang]
	if cui == nil {
		cui = &apb.KzipInfo_CorpusInfo_CompilationUnits{}
		c.LanguageCompilationUnits[lang] = cui
	}
	return cui
}

func requiredInputInfo(corpus, lang string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo_RequiredInputs {
	c := corpusInfo(corpus, kzipInfo)
	lri := c.LanguageRequiredInputs[lang]
	if lri == nil {
		lri = &apb.KzipInfo_CorpusInfo_RequiredInputs{}
		c.LanguageRequiredInputs[lang] = lri
	}
	return lri
}

func sourceInfo(corpus, lang string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo_RequiredInputs {
	c := corpusInfo(corpus, kzipInfo)
	ls := c.LanguageSources[lang]
	if ls == nil {
		ls = &apb.KzipInfo_CorpusInfo_RequiredInputs{}
		c.LanguageSources[lang] = ls
	}
	return ls
}

func corpusInfo(corpus string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo {
	i := kzipInfo.GetCorpora()[corpus]
	if i == nil {
		i = &apb.KzipInfo_CorpusInfo{
			LanguageCompilationUnits: make(map[string]*apb.KzipInfo_CorpusInfo_CompilationUnits),
			LanguageRequiredInputs:   make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
			LanguageSources:          make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
		}
		kzipInfo.Corpora[corpus] = i
	}
	return i
}
