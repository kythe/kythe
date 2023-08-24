/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

package markedsource

import (
	"fmt"

	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/edges"
	"kythe.io/kythe/go/util/schema/facts"

	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// Resolver produces fully resolved MarkedSources from a set of entries.
type Resolver struct {
	nodes map[string]*cpb.MarkedSource

	params  map[string][]string
	tparams map[string][]string
	typed   map[string]string
}

// NewResolver constructs a MarkedSource resolver from the given entries.
func NewResolver(entries []*spb.Entry) (*Resolver, error) {
	r := &Resolver{
		nodes:   make(map[string]*cpb.MarkedSource),
		params:  make(map[string][]string),
		tparams: make(map[string][]string),
		typed:   make(map[string]string),
	}
	for _, e := range entries {
		if e.GetFactName() == facts.Code {
			ticket := kytheuri.ToString(e.GetSource())
			var ms cpb.MarkedSource
			if err := proto.Unmarshal(e.GetFactValue(), &ms); err != nil {
				return nil, fmt.Errorf("error unmarshalling code for %s: %v", ticket, err)
			}
			r.nodes[ticket] = &ms
		} else if e.GetEdgeKind() != "" {
			ticket := kytheuri.ToString(e.GetSource())
			kind, ord, _ := edges.ParseOrdinal(e.GetEdgeKind())
			if ord < 0 {
				return nil, fmt.Errorf("invalid ordinal: %d", ord)
			}
			switch kind {
			case edges.Typed:
				r.typed[ticket] = kytheuri.ToString(e.GetTarget())
			case edges.Param:
				params := r.params[ticket]
				if len(params)-1 < ord {
					n := make([]string, ord+1)
					copy(n, params)
					params = n
					r.params[ticket] = params
				}
				params[ord] = kytheuri.ToString(e.GetTarget())
			case edges.TParam:
				tparams := r.tparams[ticket]
				if len(tparams)-1 < ord {
					n := make([]string, ord+1)
					copy(n, tparams)
					tparams = n
					r.tparams[ticket] = tparams
				}
				tparams[ord] = kytheuri.ToString(e.GetTarget())
			}
		}
	}
	return r, nil
}

// Resolve returns the fully resolved MarkedSource for the given source VName.
// May return nil if no MarkedSource is found.
func (r *Resolver) Resolve(src *spb.VName) *cpb.MarkedSource {
	return r.ResolveTicket(kytheuri.ToString(src))
}

// ResolveTicket returns the fully resolved MarkedSource for the given source ticket.
// May return nil if no MarkedSource is found.
func (r *Resolver) ResolveTicket(ticket string) *cpb.MarkedSource {
	if ticket == "" {
		return nil
	}
	return r.resolve(ticket, r.nodes[ticket])
}

func ensureKind(ms *cpb.MarkedSource, k cpb.MarkedSource_Kind) *cpb.MarkedSource {
	if ms.GetKind() == k {
		return ms
	}
	if ms != nil && ms.GetKind() == cpb.MarkedSource_BOX {
		ret := proto.Clone(ms).(*cpb.MarkedSource)
		ret.Kind = k
		return ret
	}
	return &cpb.MarkedSource{
		Kind:  k,
		Child: []*cpb.MarkedSource{ms},
	}
}

var invalidLookupReplacement = &cpb.MarkedSource{PreText: "???"}

func (r *Resolver) resolve(ticket string, ms *cpb.MarkedSource) *cpb.MarkedSource {
	if ms == nil {
		return ms
	}
	switch ms.GetKind() {
	case cpb.MarkedSource_LOOKUP_BY_PARAM:
		params := r.params[ticket]
		if int(ms.GetLookupIndex()) >= len(params) {
			return ensureKind(invalidLookupReplacement, cpb.MarkedSource_PARAMETER)
		}
		if p := params[ms.GetLookupIndex()]; p != "" {
			return ensureKind(r.ResolveTicket(p), cpb.MarkedSource_PARAMETER)
		}
	case cpb.MarkedSource_LOOKUP_BY_TPARAM:
		tparams := r.tparams[ticket]
		if int(ms.GetLookupIndex()) >= len(tparams) {
			return ensureKind(invalidLookupReplacement, cpb.MarkedSource_PARAMETER)
		}
		if p := tparams[ms.GetLookupIndex()]; p != "" {
			return ensureKind(r.ResolveTicket(p), cpb.MarkedSource_PARAMETER)
		}
	case cpb.MarkedSource_LOOKUP_BY_TYPED:
		return ensureKind(r.ResolveTicket(r.typed[ticket]), cpb.MarkedSource_TYPE)
	case cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS, cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM:
		// TODO: handle param/default
		params := r.params[ticket]
		if int(ms.GetLookupIndex()) > len(params) {
			return ensureKind(invalidLookupReplacement, cpb.MarkedSource_PARAMETER)
		}
		return r.resolveParameters(ms, params[ms.GetLookupIndex():])
	case cpb.MarkedSource_PARAMETER_LOOKUP_BY_TPARAM:
		tparams := r.tparams[ticket]
		if int(ms.GetLookupIndex()) > len(tparams) {
			return ensureKind(invalidLookupReplacement, cpb.MarkedSource_PARAMETER)
		}
		return r.resolveParameters(ms, tparams[ms.GetLookupIndex():])
	}
	return r.resolveChildren(ticket, ms)
}

func (r *Resolver) resolveParameters(base *cpb.MarkedSource, params []string) *cpb.MarkedSource {
	n := proto.Clone(base).(*cpb.MarkedSource)
	n.LookupIndex = 0
	n.Kind = cpb.MarkedSource_PARAMETER
	n.Child = make([]*cpb.MarkedSource, len(params))
	for i, p := range params {
		n.Child[i] = r.ResolveTicket(p)
	}
	return n
}

func (r *Resolver) resolveChildren(ticket string, ms *cpb.MarkedSource) *cpb.MarkedSource {
	n := proto.Clone(ms).(*cpb.MarkedSource)
	children := make([]*cpb.MarkedSource, len(n.GetChild()))
	for i, ms := range n.GetChild() {
		children[i] = r.resolve(ticket, ms)
	}
	n.Child = children
	return n
}
