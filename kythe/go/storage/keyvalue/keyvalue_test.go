/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

package keyvalue

import (
	"bytes"
	"strings"
	"testing"

	"google.golang.org/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

func TestKeyRange(t *testing.T) {
	r := KeyRange([]byte("key"))
	if c := bytes.Compare(r.Start, r.End); c != -1 {
		t.Errorf("%s >= %s", r.Start, r.End)
	}
}

func TestVNameEncoding(t *testing.T) {
	tests := []*spb.VName{
		nil,
		vname("sig", "corpus", "root", "path", "language"),
		vname("", "", "", "", ""),
		vname("", "kythe", "", "", "java"),
	}

	for _, v := range tests {
		rec, err := encodeVName(v)
		if err != nil {
			t.Errorf("encodeVName: unexpected error: %v", err)
		}

		res, err := decodeVName(string(rec))
		if err != nil {
			t.Errorf("decodeVName: unexpected error: %v", err)
		}

		if !proto.Equal(res, v) {
			t.Errorf("Decoded VName doesn't match original\n  orig: %+v\ndecoded: %+v", v, res)
		}
	}
}

func TestErrors(t *testing.T) {
	tests := []struct {
		entry *spb.Entry
		error string
	}{
		{
			entry(nil, "", nil, "fact", ""),
			"missing source VName",
		},
		{
			entry(vname("sig", "corpus", "root", "path", "language"), "edgeKind", nil, "fact", ""),
			"edgeKind and target Ticket must be both non-empty or empty",
		},
		{
			entry(vname("sig", "corpus", "root", "path", "language"),
				entryKeySepStr, vname("sig2", "", "", "", ""),
				"fact", "value"),
			"edgeKind contains key separator",
		},
		{
			entry(vname("sig", "corpus", "root", "path", "language"), "", nil, entryKeySepStr, ""),
			"factName contains key separator",
		},
		{
			entry(vname("sig", entryKeySepStr, "root", "path", "language"), "", nil, "fact", ""),
			"source VName contains key separator",
		},
		{
			entry(vname("sig", "corpus", "root", "path", "language"),
				"edgeKind", vname("sig2", entryKeySepStr, "", "", ""),
				"fact", "value"),
			"target VName contains key separator",
		},
	}

	for _, test := range tests {
		key, err := EncodeKey(test.entry.Source, test.entry.FactName, test.entry.EdgeKind, test.entry.Target)
		if err == nil {
			t.Fatalf("Missing expected error containing %q for test {%+v}; got %q", test.error, test.entry, string(key))
		} else if !strings.Contains(err.Error(), test.error) {
			t.Errorf("Expected error containing %q; received %v from {%+v}", test.error, err, test.entry)
		}
	}
}

func TestKeyEncoding(t *testing.T) {
	tests := []*spb.Entry{
		entry(vname("sig", "corpus", "root", "path", "language"), "", nil, "fact", "value"),
		entry(vname("sig", "corpus", "root", "path", "language"),
			"someEdge", vname("anotherVName", "", "", "", ""),
			"/", ""),
		entry(vname(entryKeyPrefix, "~!@#$%^&*()_+`-={}|:;\"'?/>.<,", "", "", ""), "", nil, "/", ""),
	}

	for _, test := range tests {
		key, err := EncodeKey(test.Source, test.FactName, test.EdgeKind, test.Target)
		fatalOnErr(t, "Error encoding key: %v", err)

		if !bytes.HasPrefix(key, entryKeyPrefixBytes) {
			t.Fatalf("Key missing entry prefix: %q", string(key))
		}

		prefix, err := KeyPrefix(test.Source, test.EdgeKind)
		fatalOnErr(t, "Error creating key prefix: %v", err)

		if !bytes.HasPrefix(key, prefix) {
			t.Fatalf("Key missing KeyPrefix: %q %q", string(key), string(prefix))
		}

		entry, err := Entry(key, test.FactValue)
		fatalOnErr(t, "Error creating Entry from key: %v", err)

		if !proto.Equal(entry, test) {
			t.Errorf("Expected Entry: {%+v}; Got: {%+v}", test, entry)
		}
	}
}

func fatalOnErr(t *testing.T, msg string, err error) {
	if err != nil {
		t.Fatalf(msg, err)
	}
}

func vname(signature, corpus, root, path, language string) *spb.VName {
	return &spb.VName{
		Signature: signature,
		Corpus:    corpus,
		Root:      root,
		Path:      path,
		Language:  language,
	}
}

func entry(src *spb.VName, edgeKind string, target *spb.VName, factName string, factValue string) *spb.Entry {
	return &spb.Entry{
		Source:    src,
		EdgeKind:  edgeKind,
		Target:    target,
		FactName:  factName,
		FactValue: []byte(factValue),
	}
}
