/*
 * Copyright 2018 Google Inc. All rights reserved.
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

package riegeli_test

import (
	"encoding/hex"
	"io"
	"os"
	"testing"

	"kythe.io/kythe/go/storage/stream"
	"kythe.io/kythe/go/util/riegeli"

	"github.com/golang/protobuf/proto"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

var (
	goldenJSONFile    = "testdata/golden.entries.json"
	goldenRiegeliFile = "testdata/golden.entries.riegeli"
)

type jsonReader struct{ ch <-chan *spb.Entry }

func (j *jsonReader) Next() (*spb.Entry, error) {
	e, ok := <-j.ch
	if !ok {
		return nil, io.EOF
	}
	return e, nil
}

func TestGoldenTestData(t *testing.T) {
	jsonFile, err := os.Open(goldenJSONFile)
	if err != nil {
		t.Fatal(err)
	}
	defer jsonFile.Close()
	riegeliFile, err := os.Open(goldenRiegeliFile)
	if err != nil {
		t.Fatal(err)
	}
	defer riegeliFile.Close()

	jsonReader := &jsonReader{stream.ReadJSONEntries(jsonFile)}
	riegeliReader := riegeli.NewReader(riegeliFile)

	for {
		expected, err := jsonReader.Next()
		if err == io.EOF {
			if rec, err := riegeliReader.Next(); err != io.EOF {
				t.Fatalf("Unexpected error/record at end of Riegeli file: %q %v", hex.EncodeToString(rec), err)
			}
			return
		} else if err != nil {
			t.Fatalf("Error reading JSON golden data: %v", err)
		}

		found := &spb.Entry{}
		if err := riegeliReader.NextProto(found); err != nil {
			t.Fatalf("Error reading Riegeli golden data: %v", err)
		}

		if !proto.Equal(expected, found) {
			t.Errorf("Unexpected record: found: {%+v}; expected: {%+v}", found, expected)
		}
	}
}
