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
	"path/filepath"
	"strconv"
	"strings"

	"kythe.io/kythe/go/platform/kzip"
	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
	"google.golang.org/protobuf/encoding/prototext"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// KzipInfo scans the kzip in f and counts contained files and units, giving a
// breakdown by corpus and language. It also records the size (in bytes) of the
// kzip specified by fileSize in the returned KzipInfo. This is a convenience
// method and thin wrapper over the Accumulator. If you need to do more than
// just calculate KzipInfo while doing a kzip.Scan(), you should use the
// Accumulator directly.
func KzipInfo(f kzip.File, fileSize int64, scanOpts ...kzip.ScanOption) (*apb.KzipInfo, error) {
	a := NewAccumulator(fileSize)
	if err := kzip.Scan(f, func(r *kzip.Reader, unit *kzip.Unit) error {
		a.Accumulate(unit)
		return nil
	}, scanOpts...); err != nil {
		return nil, fmt.Errorf("scanning kzip: %v", err)
	}
	return a.Get(), nil
}

// Accumulator is used to build a summary of a collection of compilation units.
// Usage:
//
//	a := NewAccumulator(fileSize)
//	a.Accumulate(unit) // call for each compilation unit
//	info := a.Get()    // get the resulting KzipInfo
type Accumulator struct {
	KzipInfo *apb.KzipInfo
}

// NewAccumulator creates a new Accumulator instance given the kzip fileSize (in
// bytes).
func NewAccumulator(fileSize int64) *Accumulator {
	return &Accumulator{
		KzipInfo: &apb.KzipInfo{
			Corpora: make(map[string]*apb.KzipInfo_CorpusInfo),
			Size:    fileSize,
		},
	}
}

// Accumulate should be called for each unit in the kzip so its counts can be
// recorded.
func (a *Accumulator) Accumulate(u *kzip.Unit) {
	// Set of canonicalized source file paths in the kzip
	srcs := stringset.New()
	for _, p := range u.Proto.SourceFile {
		srcs.Add(filepath.Clean(p))
	}

	cuLang := u.Proto.GetVName().GetLanguage()
	if cuLang == "" {
		msg := fmt.Sprintf("CU(%s) does not specify a language", formatVName(u.Proto.GetVName()))
		a.KzipInfo.CriticalKzipErrors = append(a.KzipInfo.CriticalKzipErrors, msg)
		return
	}

	cuInfo(u.Proto.GetVName().GetCorpus(), cuLang, a.KzipInfo).Count++

	if cuLang == "java" {
		v := javaSourceVersion(u.Proto.GetArgument())
		cuInfo(u.Proto.GetVName().GetCorpus(), cuLang, a.KzipInfo).JavaVersionCount[int32(v)]++
	}

	var srcCorpora stringset.Set
	var absPaths stringset.Set
	srcsWithRI := stringset.New()
	for _, ri := range u.Proto.RequiredInput {
		if strings.HasPrefix(ri.GetVName().GetPath(), "/") && !strings.HasPrefix(ri.GetVName().GetPath(), "/kythe_builtins/") {
			absPaths.Add(ri.GetVName().GetPath())
		}

		riCorpus := requiredInputCorpus(u, ri)
		if riCorpus == "" {
			// Trim spaces to work around the fact that log("%v", proto) is inconsistent about trailing spaces in google3 vs open-source go.
			msg := strings.TrimSpace(fmt.Sprintf("unable to determine corpus for required_input %q in CU(%s)", ri.Info.Path, formatVName(u.Proto.GetVName())))
			a.KzipInfo.CriticalKzipErrors = append(a.KzipInfo.CriticalKzipErrors, msg)
			return
		}
		requiredInputInfo(riCorpus, cuLang, a.KzipInfo).Count++
		// canonicalize required_input path before checking against source
		// files. In some cases, required_input paths may be non-canonical due
		// to compiler idiosyncrasies (ahem c++), but it's ok to canonicalize
		// for the purposes of this validation check.
		normalizedInputPath := filepath.Clean(ri.Info.Path)
		if srcs.Contains(normalizedInputPath) {
			sourceInfo(riCorpus, cuLang, a.KzipInfo).Count++
			srcCorpora.Add(riCorpus)
			srcsWithRI.Add(normalizedInputPath)
		}
	}
	srcsWithoutRI := srcs.Diff(srcsWithRI)
	for path := range srcsWithoutRI {
		msg := fmt.Sprintf("source %q in CU(%s) doesn't have a required_input entry", path, formatVName(u.Proto.GetVName()))
		a.KzipInfo.CriticalKzipErrors = append(a.KzipInfo.CriticalKzipErrors, msg)
	}
	if srcCorpora.Len() != 1 {
		// This is a warning for now, but may become an error.
		log.Infof("Multiple corpora in unit. unit vname={%v}; src corpora=%v; srcs=%v", u.Proto.GetVName(), srcCorpora, u.Proto.SourceFile)
	}

	a.KzipInfo.AbsolutePaths = absPaths.Elements()
}

// javaSourceVersion checks if the CU args include setting source to a language level and if it does
// it returns that version. If no source flag is set or it can't be read, it returns 0.
func javaSourceVersion(args []string) int {
	for i, arg := range args {
		if arg == "-source" || arg == "--source" {
			if i+1 < len(args) {
				version := args[i+1]
				version = strings.TrimPrefix(version, "1.")
				intVersion, err := strconv.Atoi(version)
				if err != nil {
					log.Errorf("Unable to parse java version string: %v", err)
					return 0
				}
				return intVersion
			}
		}
	}
	return 0
}

// Get returns the final KzipInfo after info from each unit in the kzip has been
// accumulated.
func (a *Accumulator) Get() *apb.KzipInfo {
	return a.KzipInfo
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
func KzipInfoTotalCount(infos []*apb.KzipInfo) *apb.KzipInfo_CorpusInfo {
	totals := &apb.KzipInfo_CorpusInfo{
		LanguageRequiredInputs: make(map[string]*apb.KzipInfo_CorpusInfo_Inputs),
		LanguageSources:        make(map[string]*apb.KzipInfo_CorpusInfo_Inputs),
		LanguageCuInfo:         make(map[string]*apb.KzipInfo_CorpusInfo_CUInfo),
	}
	for _, info := range infos {
		for _, i := range info.GetCorpora() {
			for lang, stats := range i.GetLanguageRequiredInputs() {
				total := totals.LanguageRequiredInputs[lang]
				if total == nil {
					total = &apb.KzipInfo_CorpusInfo_Inputs{}
					totals.LanguageRequiredInputs[lang] = total
				}
				total.Count += stats.GetCount()
			}
			for lang, stats := range i.GetLanguageSources() {
				total := totals.LanguageSources[lang]
				if total == nil {
					total = &apb.KzipInfo_CorpusInfo_Inputs{}
					totals.LanguageSources[lang] = total
				}
				total.Count += stats.GetCount()
			}
			for lang, stats := range i.GetLanguageCuInfo() {
				total := totals.LanguageCuInfo[lang]
				if total == nil {
					total = makeCUInfo()
					totals.LanguageCuInfo[lang] = total
				}
				total.Count += stats.GetCount()
				for version, count := range stats.GetJavaVersionCount() {
					total.JavaVersionCount[version] += count
				}
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
			for lang, cu := range cinfo.GetLanguageCuInfo() {
				c := cuInfo(corpus, lang, kzipInfo)
				c.Count += cu.GetCount()
				for version, count := range cu.GetJavaVersionCount() {
					c.JavaVersionCount[version] += count
				}
			}
		}
		kzipInfo.CriticalKzipErrors = append(kzipInfo.GetCriticalKzipErrors(), i.GetCriticalKzipErrors()...)
		kzipInfo.Size += i.Size
		kzipInfo.AbsolutePaths = stringset.New(kzipInfo.AbsolutePaths...).Union(stringset.New(i.AbsolutePaths...)).Elements()
	}
	return kzipInfo
}

func requiredInputInfo(corpus, lang string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo_Inputs {
	c := corpusInfo(corpus, kzipInfo)
	lri := c.LanguageRequiredInputs[lang]
	if lri == nil {
		lri = &apb.KzipInfo_CorpusInfo_Inputs{}
		c.LanguageRequiredInputs[lang] = lri
	}
	return lri
}

func sourceInfo(corpus, lang string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo_Inputs {
	c := corpusInfo(corpus, kzipInfo)
	ls := c.LanguageSources[lang]
	if ls == nil {
		ls = &apb.KzipInfo_CorpusInfo_Inputs{}
		c.LanguageSources[lang] = ls
	}
	return ls
}

func cuInfo(corpus, lang string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo_CUInfo {
	c := corpusInfo(corpus, kzipInfo)
	cuInfo := c.LanguageCuInfo[lang]
	if cuInfo == nil {
		cuInfo = makeCUInfo()
		c.LanguageCuInfo[lang] = cuInfo
	}
	return cuInfo
}

func makeCUInfo() *apb.KzipInfo_CorpusInfo_CUInfo {
	return &apb.KzipInfo_CorpusInfo_CUInfo{
		JavaVersionCount: make(map[int32]int32),
	}
}

func corpusInfo(corpus string, kzipInfo *apb.KzipInfo) *apb.KzipInfo_CorpusInfo {
	i := kzipInfo.GetCorpora()[corpus]
	if i == nil {
		i = &apb.KzipInfo_CorpusInfo{
			LanguageRequiredInputs: make(map[string]*apb.KzipInfo_CorpusInfo_Inputs),
			LanguageSources:        make(map[string]*apb.KzipInfo_CorpusInfo_Inputs),
			LanguageCuInfo:         make(map[string]*apb.KzipInfo_CorpusInfo_CUInfo),
		}
		kzipInfo.Corpora[corpus] = i
	}
	return i
}

func formatVName(v *spb.VName) string {
	return strings.ReplaceAll(prototext.MarshalOptions{}.Format(v), "  ", " ")
}
