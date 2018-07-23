/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

package riegeli

import (
	"io"
	"testing"
)

// TODO(schroederc): add more tests once an encoder is available

// TODO(schroederc): test non-proto records support
// TODO(schroederc): test deprecated protobuf "groups" support

func TestBackwardWriter(t *testing.T) {
	pieces := []string{"a", "b", "ccc", "dd", "eee"}

	var w backwardWriter
	var expectedSize int
	for _, p := range pieces {
		w.Push([]byte(p))
		expectedSize += len(p)
		if found := w.Len(); expectedSize != found {
			t.Errorf("Found w.Len() == %d; expected: %d", found, expectedSize)
		}
	}

	foundBytes := make([]byte, expectedSize)

	// Read a partial chunk
	if n, err := w.Read(foundBytes[:2]); err != nil || n != 2 {
		t.Fatalf("Failed to read partial chunk: %d %v", n, err)
	} else if found := w.Len(); found != expectedSize-2 {
		t.Errorf("Expected w.Len() == %d; found: %d", expectedSize-2, found)
	}

	// Read rest
	if n, err := io.ReadFull(&w, foundBytes[2:]); err != nil || n != expectedSize-2 {
		t.Fatalf("Failed to read full buffer: %d %v", n, err)
	} else if found := w.Len(); found != 0 {
		t.Errorf("Expected w.Len() == 0; found: %d", found)
	}

	var expected string
	for i := len(pieces) - 1; i >= 0; i-- {
		expected += pieces[i]
	}
	if found := string(foundBytes); expected != found {
		t.Fatalf("Found %q; expected %q", found, expected)
	}
}
