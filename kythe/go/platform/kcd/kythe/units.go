/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

// Package kythe implements the kcd.Unit interface for Kythe compilations.
package kythe // import "kythe.io/kythe/go/platform/kcd/kythe"

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"path"
	"sort"
	"strings"

	"kythe.io/kythe/go/platform/kcd"
	"kythe.io/kythe/go/util/ptypes"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	bipb "kythe.io/kythe/proto/buildinfo_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var toJSON = &protojson.MarshalOptions{UseProtoNames: true}

// Format is the format key used to denote Kythe compilations, stored
// as kythe.proto.CompilationUnit messages.
const Format = "kythe"

// Unit implements the kcd.Unit interface for Kythe compilations.
type Unit struct{ Proto *apb.CompilationUnit }

// MarshalBinary satisfies the encoding.BinaryMarshaler interface.
func (u Unit) MarshalBinary() ([]byte, error) { return proto.Marshal(u.Proto) }

// MarshalJSON satisfies the json.Marshaler interface.
func (u Unit) MarshalJSON() ([]byte, error) {
	s, err := toJSON.Marshal(u.Proto)
	if err != nil {
		return nil, err
	}
	return []byte(s), nil
}

// Index satisfies part of the kcd.Unit interface.
func (u Unit) Index() kcd.Index {
	v := u.Proto.GetVName()
	if v == nil {
		v = new(spb.VName)
	}
	idx := kcd.Index{
		Corpus:   v.Corpus,
		Language: v.Language,
		Output:   u.Proto.OutputKey,
		Sources:  u.Proto.SourceFile,
		Target:   v.Signature,
	}
	for _, ri := range u.Proto.RequiredInput {
		if info := ri.Info; info != nil {
			idx.Inputs = append(idx.Inputs, info.Digest)
		}
	}
	for _, detail := range u.Proto.Details {
		var info bipb.BuildDetails
		if err := ptypes.UnmarshalAny(detail, &info); err == nil {
			idx.Target = info.BuildTarget
		}
	}
	return idx
}

// LookupVName satisfies part of the kcd.Unit interface.
func (u Unit) LookupVName(inputPath string) *spb.VName {
	inputPath = u.pathKey(inputPath)
	for _, ri := range u.Proto.RequiredInput {
		if ri.Info == nil {
			continue
		}
		qp := u.pathKey(ri.Info.GetPath())
		if qp == inputPath {
			v := proto.Clone(ri.GetVName()).(*spb.VName)
			if v.GetCorpus() == "" {
				v.Corpus = u.Proto.GetVName().GetCorpus()
				v.Root = u.Proto.GetVName().GetRoot()
			}
			if v.GetPath() == "" {
				v.Path = inputPath
			}
			return v
		}
	}
	return nil
}

// Canonicalize satisfies part of the kcd.Unit interface.  It orders required
// inputs by the digest of their contents, orders environment variables and
// source paths by name, and orders compilation details by their type URL.
func (u Unit) Canonicalize() {
	pb := u.Proto

	pb.RequiredInput = sortAndDedup(pb.RequiredInput)
	sort.Sort(byName(pb.Environment))
	sort.Strings(pb.SourceFile)
	ptypes.SortByTypeURL(pb.Details)
}

// Digest satisfies part of the kcd.Unit interface.
func (u Unit) Digest() string {
	sha := sha256.New()
	pb := u.Proto
	if pb == nil {
		pb = new(apb.CompilationUnit)
	}
	put := func(tag string, ss ...string) {
		fmt.Fprintln(sha, tag)
		for _, s := range ss {
			fmt.Fprint(sha, s, "\x00")
		}
	}
	putv := func(tag string, v *spb.VName) {
		put(tag, v.GetSignature(), v.GetCorpus(), v.GetRoot(), v.GetPath(), v.GetLanguage())
	}
	putv("CU", pb.VName)
	for _, ri := range pb.RequiredInput {
		putv("RI", ri.VName)
		put("IN", ri.Info.GetPath(), ri.Info.GetDigest())
	}
	put("ARG", pb.Argument...)
	put("OUT", pb.OutputKey)
	put("SRC", pb.SourceFile...)
	put("CWD", pb.WorkingDirectory)
	put("CTX", pb.EntryContext)
	for _, env := range pb.Environment {
		put("ENV", env.Name, env.Value)
	}
	for _, d := range pb.Details {
		put("DET", d.TypeUrl, string(d.Value))
	}
	return hex.EncodeToString(sha.Sum(nil)[:])
}

// pathKey returns a cleaned path, relative to the compilation working directory.
func (u Unit) pathKey(inputPath string) string {
	root := path.Clean(u.Proto.GetWorkingDirectory())
	if root == "" {
		root = "/"
	}

	if path.IsAbs(inputPath) {
		inputPath = path.Clean(inputPath)
	} else {
		inputPath = path.Join(root, inputPath)
	}

	var prefix string
	if root == "/" {
		prefix = root
	} else {
		prefix = root + "/"
	}
	return strings.TrimPrefix(inputPath, prefix)
}

// ConvertUnit reports whether v can be converted to a Kythe kcd.Unit, and if
// so returns the appropriate implementation.
func ConvertUnit(v any) (kcd.Unit, bool) {
	if u, ok := v.(*apb.CompilationUnit); ok {
		return Unit{u}, true
	}
	return nil, false
}

type byDigest []*apb.CompilationUnit_FileInput

func (b byDigest) Len() int           { return len(b) }
func (b byDigest) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }
func (b byDigest) Less(i, j int) bool { return compareInputs(b[i], b[j]) < 0 }

func compareInputs(a, b *apb.CompilationUnit_FileInput) int {
	if n := strings.Compare(a.Info.GetDigest(), b.Info.GetDigest()); n != 0 {
		return n
	}
	return strings.Compare(a.Info.GetPath(), b.Info.GetPath())
}

type byName []*apb.CompilationUnit_Env

func (b byName) Len() int           { return len(b) }
func (b byName) Less(i, j int) bool { return b[i].Name < b[j].Name }
func (b byName) Swap(i, j int)      { b[i], b[j] = b[j], b[i] }

func sortAndDedup(ri []*apb.CompilationUnit_FileInput) []*apb.CompilationUnit_FileInput {
	if len(ri) == 0 {
		return nil
	}
	sort.Sort(byDigest(ri))

	// Invariant: All elements at or before i are unique (no duplicates).
	i, j := 0, 1
	for j < len(ri) {
		// If ri[j] ≠ ri[i], it is a new element, not a duplicate.  Move it
		// next in sequence after i and advance i; this ensures we keep the
		// first occurrence among duplicates.
		//
		// Otherwise, ri[k] == ri[i] for i <= k <= j, i.e., we are scanning a
		// run of duplicates, and should leave i where it is.
		if compareInputs(ri[i], ri[j]) != 0 {
			i++
			if i != j {
				ri[i], ri[j] = ri[j], ri[i]
			}
		}
		j++
	}
	return ri[:i+1]
}
