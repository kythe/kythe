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
	"regexp"

	"kythe.io/kythe/go/util/kytheuri"

	cpb "kythe.io/kythe/proto/common_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

func compileCorpusPathFilters(fs *xpb.CorpusPathFilters) (*corpusPathFilter, error) {
	if len(fs.GetFilter()) == 0 {
		return nil, nil
	}
	f := &corpusPathFilter{}
	for _, filter := range fs.GetFilter() {
		p, err := compileCorpusPathFilter(filter)
		if err != nil {
			return nil, err
		}
		f.pattern = append(f.pattern, p)
	}
	return f, nil
}

func compileCorpusPathFilter(f *xpb.CorpusPathFilter) (*corpusPathPattern, error) {
	p := &corpusPathPattern{}
	if f.GetType() == xpb.CorpusPathFilter_EXCLUDE {
		p.inverse = true
	}
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
	return p, nil
}

type corpusPathPattern struct {
	corpus, root, path *regexp.Regexp

	inverse bool
}

func (p *corpusPathPattern) Allow(c *cpb.CorpusPath) bool {
	return p.inverse != ((p.corpus == nil || p.corpus.MatchString(c.GetCorpus())) &&
		(p.root == nil || p.root.MatchString(c.GetRoot())) &&
		(p.path == nil || p.path.MatchString(c.GetPath())))
}

type corpusPathFilter struct {
	pattern []*corpusPathPattern
}

func (f *corpusPathFilter) Allow(c *cpb.CorpusPath) bool {
	if f == nil || c == nil {
		return true
	}

	for _, p := range f.pattern {
		if !p.Allow(c) {
			return false
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
