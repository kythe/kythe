/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package packdb

import (
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"kythe.io/kythe/go/platform/indexpack"
	"kythe.io/kythe/go/platform/kcd/kythe"
	"kythe.io/kythe/go/platform/kcd/testutil"

	"golang.org/x/net/context"
)

func TestPackDB(t *testing.T) {
	dir, err := ioutil.TempDir("", "TestPack")
	if err != nil {
		t.Fatalf("Unable to create temp directory: %v", err)
	}
	defer os.RemoveAll(dir)
	ctx := context.Background()
	path := filepath.Join(dir, "packdb")
	pack, err := indexpack.Create(ctx, path, indexpack.UnitType(testutil.UnitType))
	if err != nil {
		t.Fatalf("Unable to create index pack %q: %v", path, err)
	}
	t.Logf("Created index pack at %q", path)

	db := &DB{
		Archive:   pack,
		FormatKey: testutil.FormatKey,
		Convert:   kythe.ConvertUnit,
	}
	for _, err := range testutil.Run(ctx, db) {
		t.Error(err)
	}
}
