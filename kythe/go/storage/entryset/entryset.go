/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

// Package entryset implements a compact representation for sets of Kythe entry
// messages in the format emitted by indexers.
//
// Call New to construct an empty set, and use the Add method to add entries:
//
//   set := entryset.New(nil)
//   for entry := range readEntries() {
//      if err := set.Add(entry); err != nil {
//         log.Exitf("Invalid entry: %v", err)
//      }
//   }
//
// Entries are automatically deduplicated. You can traverse the contents of an
// entry set with the Visit method, which takes a callback:
//
//   set.Visit(func(e *storagepb.Entry) bool {
//      process(e)
//      return wantMore
//   })
//
// An entry set may or may not be "canonical": A canonical entry set has the
// property that calling its Visit method will deliver all the entries in the
// canonical entry order (http://www.kythe.io/docs/kythe-storage.html).  A
// newly-created entry set is canonical; a call to Add may invalidate this
// status. Call the Canonicalize method to canonicalize an entryset.
//
//    set := entryset.New(nil) // set is canonical
//    set.Add(e)               // set is no longer canonical
//    set.Canonicalize()       // set is (once again) canonical
//
// An entryset can be converted into a kythe.storage.EntrySet protobuf message
// using the Encode method. This message is defined in entryset.proto. You can
// construct a Set from an EntrySet message using Decode:
//
//    pb := old.Encode()
//    new, err := entryset.Decode(pb)
//    if err != nil {
//      log.Exitf("Invalid entryset message: %v", err)
//    }
//
// When rendered in wire format, the protobuf encoding is considerably more
// compact than a naive entry stream.
package entryset // import "kythe.io/kythe/go/storage/entryset"

import (
	"fmt"
	"io"
	"sort"

	"github.com/golang/protobuf/proto"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema/edges"

	espb "kythe.io/kythe/proto/entryset_go_proto"
	intpb "kythe.io/kythe/proto/internal_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// A Set represents a set of unique entries.
type Set struct {
	syms []string // lazily-initialized symbol lookup
	nids []node   // lazily-initialized node lookup

	symid map[string]id
	nodes map[node]nid
	facts map[nid]map[fact]struct{}
	edges map[nid]map[edge]struct{}
	canon bool // true if this set is canonicalized
	opts  *Options

	addCalls  int
	addErrors int
}

// Options provide configuration settings for a Set.
// A nil *Options provides sensible default values.
type Options struct {
	// When encoding a set to wire format, any symbol longer than this number
	// of bytes will be split into chunks of at most this length.
	// If ≤ 0, no splitting is performed.
	MaxSymbolBytes int
}

func (o *Options) maxSymbolBytes() int {
	if o == nil || o.MaxSymbolBytes < 0 {
		return 0
	}
	return o.MaxSymbolBytes
}

// New constructs a new Set containing no entries.
func New(opts *Options) *Set {
	s := &Set{
		symid: make(map[string]id),
		nodes: make(map[node]nid),
		facts: make(map[nid]map[fact]struct{}),
		edges: make(map[nid]map[edge]struct{}),
		opts:  opts,
	}
	s.enter("") // pre-assign "" as ID 0
	s.canon = true
	return s
}

// Stats carry statistics about the contents of a Set.
type Stats struct {
	Adds    int // number of times Add was successfully invoked
	Errors  int // number of times Add reported an error
	Nodes   int // count of unique nodes stored
	Facts   int // total number of facts stored
	Edges   int // total number of edges stored
	Symbols int // total count of unique symbols stored
}

// Stats returns current statistics for s.
func (s *Set) Stats() *Stats {
	stats := &Stats{
		Adds:    s.addCalls,
		Errors:  s.addErrors,
		Nodes:   len(s.nodes),
		Symbols: len(s.symid),
	}
	for _, facts := range s.facts {
		stats.Facts += len(facts)
	}
	for _, edges := range s.edges {
		stats.Edges += len(edges)
	}
	return stats
}

// Add adds the specified entry to the set, returning an error if the entry is
// structurally invalid. An invalid entry does not corrupt the state of the
// set, such entries are simply discarded. It is therefore safe to ignore an
// error from this method if you want to drop invalid data.
func (s *Set) Add(e *spb.Entry) error {
	if (e.Target == nil) != (e.EdgeKind == "") {
		s.addErrors++
		return fmt.Errorf("entryset: invalid entry: target=%v/kind=%v", e.Target == nil, e.EdgeKind == "")
	}
	s.addCalls++
	src := s.addVName(e.Source)
	if e.Target != nil {
		s.addEdge(src, edge{
			kind:   s.enter(e.EdgeKind),
			target: s.addVName(e.Target),
		})
	} else {
		s.addFact(src, fact{
			name:  s.enter(e.FactName),
			value: s.enter(string(e.FactValue)),
		})
	}
	return nil
}

// Sources groups the entries of the set into Source messages and invokes f for
// each one. If f returns false the visit is aborted.
func (s *Set) Sources(f func(*intpb.Source) bool) {
	for i := 0; i < len(s.nodes); i++ {
		n := s.node(nid(i))
		nid := nid(i)
		src := &intpb.Source{
			Ticket:     s.ticket(n),
			Facts:      make(map[string][]byte),
			EdgeGroups: make(map[string]*intpb.Source_EdgeGroup),
		}
		for fact := range s.facts[nid] {
			src.Facts[s.symbol(fact.name)] = []byte(s.symbol(fact.value))
		}
		for edge := range s.edges[nid] {
			kind, ordinal, _ := edges.ParseOrdinal(s.symbol(edge.kind))
			eg := src.EdgeGroups[kind]
			if eg == nil {
				eg = new(intpb.Source_EdgeGroup)
				src.EdgeGroups[kind] = eg
			}
			eg.Edges = append(eg.Edges, &intpb.Source_Edge{
				Ticket:  s.ticket(s.node(edge.target)),
				Ordinal: int32(ordinal),
			})
		}
		if !f(src) {
			return // early exit
		}
	}
}

// Visit calls f with each entry in the set. If s is canonical, entries are
// delivered in canonical order; otherwise the order is unspecified.  If f
// returns false the visit is aborted.
func (s *Set) Visit(f func(*spb.Entry) bool) {
	// Scan nodes in order by nid. If s is canonical, they will also be
	// lexicographically ordered.
	for i := 0; i < len(s.nodes); i++ {
		n := s.node(nid(i))
		src := s.vname(n)
		nid := nid(i)

		// Deliver facts. If s is canonical, they will be correctly ordered.
		facts := sortedFacts(s.facts[nid])
		for _, fact := range facts {
			if !f(&spb.Entry{
				Source:    src,
				FactName:  s.symbol(fact.name),
				FactValue: []byte(s.symbol(fact.value)),
			}) {
				return // early exit
			}
		}

		// Deliver edges. If s is canonical, they will be correctly ordered.
		edges := sortedEdges(s.edges[nid])
		for _, edge := range edges {
			tgt := s.vname(s.node(edge.target))
			if !f(&spb.Entry{
				Source:   src,
				Target:   tgt,
				EdgeKind: s.symbol(edge.kind),
			}) {
				return // early exit
			}
		}
	}
}

// Canonicalize modifies s in-place to be in canonical form, and returns s to
// permit chaining.  If s is already in canonical form, it is returned
// unmodified.
func (s *Set) Canonicalize() *Set {
	if s.canon {
		return s
	}

	// Copy out the symbol table and order it lexicographically, keeping track
	// of the inverse permutation. The inverse will give us what we need to
	// remap the rest of the data.
	syms := make([]string, len(s.symid))
	for sym, id := range s.symid {
		syms[int(id)] = sym
	}
	sinv := sortInverse(sort.StringSlice(syms))
	smap := func(i id) id { return id(sinv[int(i)]) }

	// Set up the new symbol table...
	out := New(s.opts)
	out.addCalls = s.addCalls
	out.addErrors = s.addErrors
	for i, sym := range syms {
		if id := out.putsym(sym); int(id) != i {
			panic("symbol table corrupted")
		}
	}

	// Copy out the nodes table and rewrite all the values in terms of the
	// remapped symbol table.
	nodes := make([]node, len(s.nodes))
	for n, nid := range s.nodes {
		nodes[int(nid)] = node{
			signature: smap(n.signature),
			corpus:    smap(n.corpus),
			root:      smap(n.root),
			path:      smap(n.path),
			language:  smap(n.language),
		}
	}
	ninv := sortInverse(byNode(nodes))
	nmap := func(n nid) nid { return nid(ninv[int(n)]) }

	// Set up the new nodes table...
	for i, node := range nodes {
		if nid := out.addNode(node); int(nid) != i {
			panic("node table corrupted")
		}
	}

	// Update all the facts...
	for oid, facts := range s.facts {
		nid := nmap(oid)
		for f := range facts {
			out.addFact(nid, fact{
				name:  smap(f.name),
				value: smap(f.value),
			})
		}
	}

	// Update all the edges...
	for oid, edges := range s.edges {
		nid := nmap(oid)
		for e := range edges {
			out.addEdge(nid, edge{
				kind:   smap(e.kind),
				target: nmap(e.target),
			})
		}
	}
	*s = *out
	s.canon = true
	return s
}

// Encode constructs a canonical version of s and renders it into a
// kythe.storage.EntrySet protobuf message.
func (s *Set) Encode() *espb.EntrySet {
	s.Canonicalize()
	es := &espb.EntrySet{
		Nodes:      make([]*espb.EntrySet_Node, len(s.nodes)),
		FactGroups: make([]*espb.EntrySet_FactGroup, len(s.nodes)),
		EdgeGroups: make([]*espb.EntrySet_EdgeGroup, len(s.nodes)),
		Symbols:    make([]*espb.EntrySet_String, len(s.symid)-1), // skip ""
	}
	for i := 0; i < len(s.nodes); i++ {
		nid := nid(i)
		n := s.node(nid)
		es.Nodes[i] = &espb.EntrySet_Node{
			Corpus:    int32(n.corpus),
			Language:  int32(n.language),
			Path:      int32(n.path),
			Root:      int32(n.root),
			Signature: int32(n.signature),
		}

		facts := sortedFacts(s.facts[nid])
		es.FactGroups[i] = &espb.EntrySet_FactGroup{
			Facts: make([]*espb.EntrySet_Fact, len(facts)),
		}
		for j, f := range facts {
			es.FactGroups[i].Facts[j] = &espb.EntrySet_Fact{
				Name:  int32(f.name),
				Value: int32(f.value),
			}
		}

		edges := sortedEdges(s.edges[nid])
		es.EdgeGroups[i] = &espb.EntrySet_EdgeGroup{
			Edges: make([]*espb.EntrySet_Edge, len(edges)),
		}
		for j, e := range edges {
			es.EdgeGroups[i].Edges[j] = &espb.EntrySet_Edge{
				Kind:   int32(e.kind),
				Target: int32(e.target),
			}
		}
	}

	// Pack the string table.
	prev := ""
	for i := 1; i < len(s.symid); i++ { // start at 1 to skip ""
		sym := s.symbol(id(i))
		pfx := lcp(prev, sym)
		es.Symbols[i-1] = &espb.EntrySet_String{
			Prefix: int32(pfx),
			Suffix: []byte(sym[pfx:]),
		}
		prev = sym
	}
	return es
}

// WriteTo writes s to w as a wire-format EntrySet message.
func (s *Set) WriteTo(w io.Writer) (int64, error) {
	bits, err := proto.Marshal(s.Encode())
	if err != nil {
		return 0, err
	}
	nw, err := w.Write(bits)
	return int64(nw), err
}

// Unmarshal unmarshals a wire-format EntrySet message into a *Set.
func Unmarshal(data []byte) (*Set, error) {
	var es espb.EntrySet
	if err := proto.Unmarshal(data, &es); err != nil {
		return nil, err
	}
	return Decode(&es)
}

// Decode constructs a set from a protobuf representation.
// The resulting set will be canonical if the encoding was; if the message was
// encoded by the Encode method of a *Set it will be so.
func Decode(es *espb.EntrySet) (*Set, error) {
	s := New(nil)

	// Sanity checks: There must be equal numbers of nodes, fact groups, and
	// edge groups in the message. This simplifies the scanning logic below.
	n := len(es.Nodes)
	if len(es.FactGroups) != n || len(es.EdgeGroups) != n {
		return nil, fmt.Errorf("entryset: invalid counts: %d nodes, %d fact groups, %d edge groups",
			n, len(es.FactGroups), len(es.EdgeGroups))
	}

	// Unpack the string table. The empty string was already added by New.
	prev := ""
	for i, sym := range es.Symbols {
		n := int(sym.Prefix)
		if n > len(prev) {
			return nil, fmt.Errorf("entryset: invalid symbol table: prefix length %d > %d", n, len(prev))
		}
		cur := prev[:n] + string(sym.Suffix)
		if sid := s.enter(cur); int(sid) != i+1 {
			return nil, fmt.Errorf("entryset: symbol index error %d ≠ %d", sid, i+1)
		}
		prev = cur
	}

	// Unpack the nodes, facts, and edges.
	for i := 0; i < n; i++ {
		new := node{
			corpus:    id(es.Nodes[i].Corpus),
			language:  id(es.Nodes[i].Language),
			path:      id(es.Nodes[i].Path),
			root:      id(es.Nodes[i].Root),
			signature: id(es.Nodes[i].Signature),
		}
		if err := s.checkBounds(new.corpus, new.language, new.path, new.root, new.signature); err != nil {
			return nil, err
		}
		cur := s.addNode(new)
		if int(cur) != i {
			return nil, fmt.Errorf("entryset: node index error: %d ≠ %d", cur, i)
		}
		for _, f := range es.FactGroups[i].GetFacts() {
			if f != nil {
				new := fact{name: id(f.Name), value: id(f.Value)}
				if err := s.checkBounds(new.name, new.value); err != nil {
					return nil, err
				}
				s.addFact(cur, new)
			}
		}
		for _, e := range es.EdgeGroups[i].GetEdges() {
			if e != nil {
				new := edge{kind: id(e.Kind), target: nid(e.Target)}
				if err := s.checkBounds(new.kind); err != nil {
					return nil, err
				} else if t := int(new.target); t < 0 || t >= n {
					return nil, fmt.Errorf("entryset: target id %d out of bounds", t)
				}
				s.addEdge(cur, new)
			}
		}
	}
	return s, nil
}

// lcp returns the length of the longest common prefix of a and b, in bytes.
func lcp(a, b string) int {
	i := 0
	for i < len(a) && i < len(b) && a[i] == b[i] {
		i++
	}
	return i
}

// sortInverse sorts v lexicographically in-place, and returns the inverse
// permutation of v, so that inv[i] is the new offset of the value that was
// originally at offset i in the unsorted collection.  Thus, v'[inv[i]] = v[i]
// for all 0 ≤ i < v.Len().
func sortInverse(v sort.Interface) (inv []int) {
	p := rperm{
		vals: v,
		perm: make([]int, v.Len()),
		inv:  make([]int, v.Len()),
	}
	for i := range p.perm {
		p.inv[i] = i
		p.perm[i] = i
	}
	sort.Sort(p)
	return p.inv
}

// rperm implements sort.Interface by dispatching to another sortable type.  In
// addition, it computes an inverse permutation mapping.
type rperm struct {
	vals sort.Interface

	perm []int // perm[i] is the original offset of the string now at offset i
	inv  []int // inv[i] is the current offset of the string originally at offset i
}

func (r rperm) Len() int           { return r.vals.Len() }
func (r rperm) Less(i, j int) bool { return r.vals.Less(i, j) }

func (r rperm) Swap(i, j int) {
	r.vals.Swap(i, j)
	r.perm[i], r.perm[j] = r.perm[j], r.perm[i]
	r.inv[r.perm[i]] = i
	r.inv[r.perm[j]] = j
}

type byNode []node

func (b byNode) Len() int           { return len(b) }
func (b byNode) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }
func (b byNode) Less(i, j int) bool { return b[i].compare(b[j]) < 0 }

type byFact []fact

func (b byFact) Len() int           { return len(b) }
func (b byFact) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }
func (b byFact) Less(i, j int) bool { return b[i].compare(b[j]) < 0 }

type byEdge []edge

func (b byEdge) Len() int           { return len(b) }
func (b byEdge) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }
func (b byEdge) Less(i, j int) bool { return b[i].compare(b[j]) < 0 }

type id int  // symbol identifier
type nid int // node identifier

type node struct {
	signature id
	corpus    id
	root      id
	path      id
	language  id
}

func (n node) compare(o node) int {
	for _, v := range [...]id{
		n.corpus - o.corpus,
		n.language - o.language,
		n.path - o.path,
		n.root - o.root,
		n.signature - o.signature,
	} {
		if v != 0 {
			return int(v)
		}
	}
	return 0
}

type fact struct{ name, value id }

func (f fact) compare(o fact) int {
	if n := f.name - o.name; n != 0 {
		return int(n)
	}
	return int(f.value - o.value)
}

// sortedFacts unpacks the keys of m and returns them canonically ordered.
func sortedFacts(m map[fact]struct{}) []fact {
	facts := make([]fact, 0, len(m))
	for f := range m {
		facts = append(facts, f)
	}
	sort.Sort(byFact(facts))
	return facts
}

type edge struct {
	kind   id
	target nid
}

func (e edge) compare(o edge) int {
	if n := e.kind - o.kind; n != 0 {
		return int(n)
	}
	return int(e.target - o.target)
}

// sortedEdges unpacks the keys of m and returns them canonically ordered.
func sortedEdges(m map[edge]struct{}) []edge {
	edges := make([]edge, 0, len(m))
	for e := range m {
		edges = append(edges, e)
	}
	sort.Sort(byEdge(edges))
	return edges
}

func (s *Set) putsym(sym string) id {
	if id, ok := s.symid[sym]; ok {
		return id
	}
	s.syms = nil    // new data resets the lookup
	s.canon = false // new data invalidates canonical form
	next := id(len(s.symid))
	s.symid[sym] = next
	return next
}

// enter adds a string to the symbol table and returns its ID.
// Duplicate symbols are given the same ID each time.
func (s *Set) enter(sym string) id {
	next := s.putsym(sym)
	if int(next) < len(s.symid)-1 {
		return next
	}

	// If the symbol exceeds the length cap, add prefixes of it to the table so
	// that prefix compression will fall under the cap. For example, if sym is
	//
	//    01234566789abcdef01234566789abcdef01234566789abc
	//    ^^^^^^^^^^^^^^^^^^^|
	//                       cap
	//
	// then we will add prefixes at multiples of cap until the last one fits:
	//
	//    01234566789abcdef01234566789abcdef01234566789abc ← sym
	//    01234566789abcdef01234566789abcdef012345
	//    01234566789abcdef012                   △ 2*cap
	//                       △ 1*cap
	//
	// When canonicalized and prefix-coded, these will collapse to:
	//
	//    01234566789abcdef012
	//    <1*cap>34566789abcdef012345
	//    <2*cap>566789abc
	//
	if cap := s.opts.maxSymbolBytes(); cap > 0 {
		for n := len(sym) / cap; n > 0; n-- {
			s.putsym(sym[:n*cap])
		}
	}
	return next
}

// symbol returns the string corresponding to the given symbol ID.
func (s *Set) symbol(id id) string {
	// If necessary, (re)initialize the lookup table.
	if s.syms == nil {
		s.syms = make([]string, len(s.symid))
		for sym, id := range s.symid {
			s.syms[int(id)] = sym
		}
	}
	return s.syms[int(id)]
}

// node returns the node corresponding to the given node ID.
func (s *Set) node(nid nid) node {
	// If necessary, (re)initialize the lookup table.
	if s.nids == nil {
		s.nids = make([]node, len(s.nodes))
		for n, nid := range s.nodes {
			s.nids[int(nid)] = n
		}
	}
	return s.nids[int(nid)]
}

// addNode adds a (possibly new) node to the set, and returns its node ID.
// Duplicate nodes are given the same ID each time.
func (s *Set) addNode(n node) nid {
	if id, ok := s.nodes[n]; ok {
		return id
	}
	s.nids = nil    // new data resets the lookup
	s.canon = false // new data invalidates canonical form
	next := nid(len(s.nodes))
	s.nodes[n] = next
	return next
}

// addVName constructs a node from v and passes it to addNode.
func (s *Set) addVName(v *spb.VName) nid {
	return s.addNode(node{
		signature: s.enter(v.Signature),
		corpus:    s.enter(v.Corpus),
		root:      s.enter(v.Root),
		path:      s.enter(v.Path),
		language:  s.enter(v.Language),
	})
}

// vname returns a VName protobuf equivalent to n.
func (s *Set) vname(n node) *spb.VName {
	return &spb.VName{
		Signature: s.symbol(n.signature),
		Corpus:    s.symbol(n.corpus),
		Path:      s.symbol(n.path),
		Root:      s.symbol(n.root),
		Language:  s.symbol(n.language),
	}
}

// ticket returns a Kythe ticket equivalent to n.
func (s *Set) ticket(n node) string {
	return (&kytheuri.URI{
		Signature: s.symbol(n.signature),
		Corpus:    s.symbol(n.corpus),
		Path:      s.symbol(n.path),
		Root:      s.symbol(n.root),
		Language:  s.symbol(n.language),
	}).String()
}

// addFact adds f as a fact related to n.
func (s *Set) addFact(n nid, f fact) {
	if s.facts[n] == nil {
		s.facts[n] = map[fact]struct{}{f: struct{}{}}
		s.canon = false
	} else if _, ok := s.facts[n][f]; !ok {
		s.facts[n][f] = struct{}{}
		s.canon = false
	}
}

// addEdge adds e as an outbound edges from n.
func (s *Set) addEdge(n nid, e edge) {
	if s.edges[n] == nil {
		s.edges[n] = map[edge]struct{}{e: struct{}{}}
		s.canon = false
	} else if _, ok := s.edges[n][e]; !ok {
		s.edges[n][e] = struct{}{}
		s.canon = false
	}
}

// checkBounds returns nil if all the symbol ids given are in bounds.  This
// requires that the symbol table has already been populated.
func (s *Set) checkBounds(ids ...id) error {
	for _, id := range ids {
		if id < 0 || int(id) >= len(s.symid) {
			return fmt.Errorf("entryset: symid %d out of bounds", id)
		}
	}
	return nil
}
