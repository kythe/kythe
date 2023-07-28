/*
 * Copyright 2022 The Kythe Authors. All rights reserved.
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

package xrefs

import (
	"math"
	"regexp"
	"regexp/syntax"

	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/stringset"
	"kythe.io/kythe/go/util/kytheuri"

	"github.com/google/codesearch/index"

	cpb "kythe.io/kythe/proto/common_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

func compileCorpusPathFilters(fs *xpb.CorpusPathFilters, pr PathResolver) (*corpusPathFilter, error) {
	if len(fs.GetFilter()) == 0 {
		return nil, nil
	}
	if pr == nil {
		pr = DefaultResolvePath
	}
	f := &corpusPathFilter{}
	for _, filter := range fs.GetFilter() {
		p, err := compileCorpusPathFilter(filter, pr)
		if err != nil {
			return nil, err
		}
		f.pattern = append(f.pattern, p)

		if filter.GetType() == xpb.CorpusPathFilter_INCLUDE_ONLY {
			f.corpusQuery, err = appendQuery(f.corpusQuery, filter.GetCorpus())
			if err != nil {
				return nil, err
			}
			f.rootQuery, err = appendQuery(f.rootQuery, filter.GetRoot())
			if err != nil {
				return nil, err
			}
			f.pathQuery, err = appendQuery(f.pathQuery, filter.GetPath())
			if err != nil {
				return nil, err
			}
			f.resolvedPathQuery, err = appendQuery(f.resolvedPathQuery, filter.GetResolvedPath())
			if err != nil {
				return nil, err
			}
		}
	}
	return f, nil
}

func appendQuery(qs []*index.Query, pattern string) ([]*index.Query, error) {
	if pattern == "" {
		return qs, nil
	}
	c, err := syntax.Parse(pattern, syntax.Perl)
	if err != nil {
		return nil, err
	}
	return append(qs, index.RegexpQuery(c)), nil
}

type pageSet struct{ KeySet stringset.Set }

func (p *pageSet) Contains(i *srvpb.PagedCrossReferences_PageIndex) bool {
	return p == nil || p.KeySet.Contains(i.GetPageKey())
}

func (f *corpusPathFilter) PageSet(set *srvpb.PagedCrossReferences) *pageSet {
	idx := set.GetPageSearchIndex()
	if idx == nil || f == nil || len(f.corpusQuery)+len(f.rootQuery)+len(f.pathQuery)+len(f.resolvedPathQuery) == 0 {
		return nil
	}

	if len(set.GetPageIndex()) >= math.MaxUint32 {
		log.Warningf("too many pages to perform index search: %d", len(set.GetPageIndex()))
		return nil
	}

	list := applyQueries(idx.GetByCorpus(), f.corpusQuery, nil)
	list = applyQueries(idx.GetByRoot(), f.rootQuery, list)
	list = applyQueries(idx.GetByPath(), f.pathQuery, list)
	list = applyQueries(idx.GetByResolvedPath(), f.resolvedPathQuery, list)

	if isAllPages(list) || len(list) == len(set.GetPageIndex()) {
		return nil
	}

	s := stringset.NewSize(len(list))
	for _, p := range list {
		s.Add(set.GetPageIndex()[p].GetPageKey())
	}

	return &pageSet{s}
}

func applyQueries(p *srvpb.PagedCrossReferences_PageSearchIndex_Postings, qs []*index.Query, restrict []uint32) []uint32 {
	if len(qs) == 0 {
		return restrict
	}
	postings := diffDecodePostings(p)
	for _, q := range qs {
		restrict = applyQuery(postings, q, restrict)
	}
	return restrict
}

func diffDecodePostings(p *srvpb.PagedCrossReferences_PageSearchIndex_Postings) postings {
	res := make(postings, len(p.GetIndex()))
	for k, v := range p.GetIndex() {
		res[k] = diffDecode(v.GetPageIndex())
	}
	return res
}

func diffDecode(s []uint32) []uint32 {
	if len(s) == 0 {
		return nil
	}
	res := make([]uint32, len(s))
	res[0] = s[0]
	for i, n := range s[1:] {
		res[i+1] = res[i] + n
	}
	return res
}

type postings map[uint32][]uint32

func tri(t string) uint32 {
	return uint32(t[0])<<16 | uint32(t[1])<<8 | uint32(t[2])
}

func isAllPages(list []uint32) bool { return len(list) == 1 && list[0] == math.MaxUint32 }

func allPagesToNil(list []uint32) []uint32 {
	if isAllPages(list) {
		return nil
	}
	return list
}

func nilToAllPages(list []uint32) []uint32 {
	if list == nil {
		return allPages
	}
	return list
}

var allPages = []uint32{math.MaxUint32}

func applyQuery(idx postings, q *index.Query, restrict []uint32) []uint32 {
	restrict = allPagesToNil(restrict)

	var list []uint32
	switch q.Op {
	case index.QNone:
		return []uint32{}
	case index.QAll:
		if restrict != nil {
			return restrict
		}
		return allPages
	case index.QAnd:
		list = restrict
		for _, t := range q.Trigram {
			list = postingAnd(idx, list, tri(t))
			if len(list) == 0 {
				return []uint32{}
			}
		}
		for _, sub := range q.Sub {
			if list == nil {
				list = restrict
			}
			list = applyQuery(idx, sub, list)
			if len(list) == 0 {
				return []uint32{}
			}
		}
	case index.QOr:
		for _, t := range q.Trigram {
			list = postingOr(idx, list, tri(t), restrict)
		}
		for _, sub := range q.Sub {
			subList := applyQuery(idx, sub, restrict)
			list = mergeOr(list, subList)
		}
	}
	return list
}

func postingList(idx postings, trigram uint32, restrict []uint32) []uint32 {
	restrict = allPagesToNil(restrict)
	ps := idx[trigram]
	if isAllPages(ps) {
		return nilToAllPages(restrict)
	}
	list := make([]uint32, 0, len(ps))
	for _, p := range ps {
		if restrict != nil {
			i := 0
			for i < len(restrict) && restrict[i] < p {
				i++
			}
			restrict = restrict[i:]
			if len(restrict) == 0 || restrict[0] != p {
				continue
			}
		}
		list = append(list, p)
	}
	return list
}

func postingAnd(idx postings, list []uint32, trigram uint32) []uint32 {
	if list == nil || isAllPages(list) {
		return postingList(idx, trigram, list)
	}

	ps := idx[trigram]
	if isAllPages(ps) {
		return nilToAllPages(list)
	}

	var l int
	res := list[:0]
	for _, p := range ps {
		for l < len(list) && list[l] < p {
			l++
		}
		if l == len(list) {
			return res
		}
		if list[l] != p {
			continue
		}
		res = append(res, p)
	}
	return res
}

func mergeOr(l1, l2 []uint32) []uint32 {
	if isAllPages(l1) || isAllPages(l2) {
		return allPages
	}
	var l []uint32
	var i, j int
	for i < len(l1) || j < len(l2) {
		switch {
		case j == len(l2) || (i < len(l1) && l1[i] < l2[j]):
			l = append(l, l1[i])
			i++
		case i == len(l1) || (j < len(l2) && l1[i] > l2[j]):
			l = append(l, l2[j])
			j++
		case l1[i] == l2[j]:
			l = append(l, l1[i])
			i++
			j++
		}
	}
	return l
}

func postingOr(idx postings, list []uint32, trigram uint32, restrict []uint32) []uint32 {
	if list == nil {
		return postingList(idx, trigram, restrict)
	} else if isAllPages(list) {
		return list
	}

	ps := idx[trigram]
	if isAllPages(ps) {
		return nilToAllPages(restrict)
	}
	restrict = allPagesToNil(restrict)

	var l int
	res := list[:0]
	for _, p := range ps {
		if restrict != nil {
			i := 0
			for i < len(restrict) && restrict[i] < p {
				i++
			}
			restrict = restrict[i:]
			if len(restrict) == 0 || restrict[0] != p {
				continue
			}
		}
		for l < len(list) && list[l] < p {
			res = append(res, list[l])
			l++
		}
		if l != len(list) && list[l] == p {
			l++
		}
		res = append(res, p)
	}
	return res
}

func compileCorpusPathFilter(f *xpb.CorpusPathFilter, pr PathResolver) (*corpusPathPattern, error) {
	p := &corpusPathPattern{pathResolver: pr}
	if f.GetType() == xpb.CorpusPathFilter_EXCLUDE {
		p.inverse = true
	}
	p.corpusSpecificFilter = f.GetCorpusSpecificFilter()
	var err error
	if corpus := f.GetCorpus(); corpus != "" {
		p.corpus, err = regexp.Compile(corpus)
		if err != nil {
			return nil, err
		}
	}
	if root := f.GetRoot(); root != "" {
		p.root, err = regexp.Compile(root)
		if err != nil {
			return nil, err
		}
	}
	if path := f.GetPath(); path != "" {
		p.path, err = regexp.Compile(path)
		if err != nil {
			return nil, err
		}
	}
	if resolvedPath := f.GetResolvedPath(); resolvedPath != "" {
		p.resolvedPath, err = regexp.Compile(resolvedPath)
		if err != nil {
			return nil, err
		}
	}
	return p, nil
}

type corpusPathPattern struct {
	corpus, root, path *regexp.Regexp

	pathResolver PathResolver
	resolvedPath *regexp.Regexp

	inverse bool

	// If true, this pattern should only be used when the corpus matches or otherwise we should
	// include the corpus in the filter like any other field.
	//
	// The list of patterns in corpusPathFilter are ANDed together and that is usually what we want.
	// However, sometimes we don't know the corpus of the data being filtered and we need to pass
	// patterns for multiple corpora. In that case, we only want to apply the pattern that is
	// applicable for the corpus the CorpusPath belongs to.
	//
	// For example, if we want to *exclude* test files, we can set this to allCorpusPatterns because if
	// any pattern matches we should remove the file. However, if we wanted to *include* test files
	// only, we should only apply the pattern for the correct corpus, we do not care if other corpora
	// would or would not allow the file. Furthermore, since their corpus wouldn't match, the would
	// always say the file should not be allowed.
	corpusSpecificFilter bool
}

func (p *corpusPathPattern) Allow(c *cpb.CorpusPath) bool {
	return p.inverse != ((p.corpus == nil || p.corpus.MatchString(c.GetCorpus())) &&
		(p.root == nil || p.root.MatchString(c.GetRoot())) &&
		(p.path == nil || p.path.MatchString(c.GetPath())) &&
		(p.resolvedPath == nil || p.resolvedPath.MatchString(p.pathResolver(c))))
}

type corpusPathFilter struct {
	pattern []*corpusPathPattern

	corpusQuery, rootQuery, pathQuery, resolvedPathQuery []*index.Query
}

func (f *corpusPathFilter) Allow(c *cpb.CorpusPath) bool {
	if f == nil || c == nil {
		return true
	}

	for _, p := range f.pattern {
		if p.corpusSpecificFilter {
			// Ignore p when the corpus does not match.
			if p.corpus != nil && p.corpus.MatchString(c.GetCorpus()) {
				if !p.Allow(c) {
					return false
				}
			}
		} else {
			if !p.Allow(c) {
				return false
			}
		}
	}
	return true
}

func (f *corpusPathFilter) AllowExpandedAnchor(a *srvpb.ExpandedAnchor) bool {
	if f == nil || a == nil {
		return true
	}
	return f.AllowTicket(a.GetTicket())
}

func (f *corpusPathFilter) AllowTicket(ticket string) bool {
	if f == nil || ticket == "" {
		return true
	}
	cp, _ := kytheuri.ParseCorpusPath(ticket)
	return f.Allow(cp)
}

func (f *corpusPathFilter) FilterGroup(grp *srvpb.PagedCrossReferences_Group) (filtered int) {
	if f == nil {
		return 0
	}

	var n int
	grp.Anchor, n = f.filterAnchors(grp.GetAnchor())
	filtered += n
	grp.ScopedReference, n = f.filterReferences(grp.GetScopedReference())
	filtered += n
	grp.RelatedNode, n = f.filterRelatedNodes(grp.GetRelatedNode())
	filtered += n
	grp.Caller, n = f.filterCallers(grp.GetCaller())
	filtered += n
	return
}

func (f *corpusPathFilter) filterAnchors(as []*srvpb.ExpandedAnchor) ([]*srvpb.ExpandedAnchor, int) {
	var j int
	for i, a := range as {
		if !f.AllowExpandedAnchor(a) {
			continue
		}
		as[j] = as[i]
		j++
	}
	return as[:j], len(as) - j
}

func (f *corpusPathFilter) filterReferences(rs []*srvpb.PagedCrossReferences_ScopedReference) ([]*srvpb.PagedCrossReferences_ScopedReference, int) {
	var j int
	for i, c := range rs {
		if !f.AllowExpandedAnchor(c.GetScope()) {
			continue
		}
		rs[j] = rs[i]
		j++
	}
	return rs[:j], len(rs) - j
}

func (f *corpusPathFilter) filterCallers(cs []*srvpb.PagedCrossReferences_Caller) ([]*srvpb.PagedCrossReferences_Caller, int) {
	var j int
	for i, c := range cs {
		if !f.AllowExpandedAnchor(c.GetCaller()) {
			continue
		}
		cs[j] = cs[i]
		j++
	}
	return cs[:j], len(cs) - j
}

func (f *corpusPathFilter) filterRelatedNodes(rs []*srvpb.PagedCrossReferences_RelatedNode) ([]*srvpb.PagedCrossReferences_RelatedNode, int) {
	var j int
	for i, r := range rs {
		if def := r.GetNode().GetDefinitionLocation().GetTicket(); (def != "" && !f.AllowTicket(def)) || (def == "" && !f.AllowTicket(r.GetNode().GetTicket())) {
			continue
		}
		rs[j] = rs[i]
		j++
	}
	return rs[:j], len(rs) - j
}
