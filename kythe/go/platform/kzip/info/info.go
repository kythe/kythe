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

// KzipInfo scans the kzip in f and counts contained files and units, giving a breakdown by corpus
// and language. It also records the size (in bytes) of the kzip specified by fileSize in the
// returned KzipInfo.
func KzipInfo(f kzip.File, fileSize int64, scanOpts ...kzip.ScanOption) (*apb.KzipInfo, error) {
	// Get file counts broken down by corpus, language.
	kzipInfo := &apb.KzipInfo{
		Corpora: make(map[string]*apb.KzipInfo_CorpusInfo),
		Size:    fileSize,
	}

	err := kzip.Scan(f, func(rd *kzip.Reader, u *kzip.Unit) error {
		srcs := stringset.New(u.Proto.SourceFile...)
		cuLang := u.Proto.GetVName().GetLanguage()
		if cuLang == "" {
			msg := fmt.Sprintf("CU does not specify a language %v", u.Proto.GetVName())
			kzipInfo.CriticalKzipErrors = append(kzipInfo.CriticalKzipErrors, msg)
			return nil
		}

		var srcCorpora stringset.Set
		srcsWithRI := stringset.New()
		for _, ri := range u.Proto.RequiredInput {
			riCorpus := requiredInputCorpus(u, ri)
			if riCorpus == "" {
				msg := fmt.Sprintf("unable to determine corpus for required_input %q in CU %v", ri.Info.Path, u.Proto.GetVName())
				kzipInfo.CriticalKzipErrors = append(kzipInfo.CriticalKzipErrors, msg)
				return nil
			}
			requiredInputInfo(riCorpus, cuLang, kzipInfo).Count++
			if srcs.Contains(ri.Info.Path) {
				sourceInfo(riCorpus, cuLang, kzipInfo).Count++
				srcCorpora.Add(riCorpus)
				srcsWithRI.Add(ri.Info.Path)
			}
		}
		srcsWithoutRI := srcs.Diff(srcsWithRI)
		for path := range srcsWithoutRI {
			msg := fmt.Sprintf("source %q in CU %v doesn't have a required_input entry", path, u.Proto.GetVName())
			kzipInfo.CriticalKzipErrors = append(kzipInfo.CriticalKzipErrors, msg)
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
		LanguageRequiredInputs: make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
		LanguageSources:        make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
	}
	for _, info := range infos {
		for _, i := range info.GetCorpora() {
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
			for lang, inputs := range cinfo.GetLanguageRequiredInputs() {
				c := requiredInputInfo(corpus, lang, kzipInfo)
				c.Count += inputs.GetCount()
			}
			for lang, sources := range cinfo.GetLanguageSources() {
				c := sourceInfo(corpus, lang, kzipInfo)
				c.Count += sources.GetCount()
			}
		}
		kzipInfo.CriticalKzipErrors = append(kzipInfo.GetCriticalKzipErrors(), i.GetCriticalKzipErrors()...)
		kzipInfo.Size += i.Size
	}
	return kzipInfo
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
			LanguageRequiredInputs: make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
			LanguageSources:        make(map[string]*apb.KzipInfo_CorpusInfo_RequiredInputs),
		}
		kzipInfo.Corpora[corpus] = i
	}
	return i
}
