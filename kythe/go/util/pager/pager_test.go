/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package pager

import (
	"testing"

	"kythe.io/kythe/go/test/testutil"

	"golang.org/x/net/context"
)

type testSet struct {
	Head   string
	Total  int
	Groups []*testGroup
	Pages  int
}

type testPage struct {
	Index int
	Key   string
	Group *testGroup
}

type testGroup struct {
	Key  string
	Vals []int
}

func TestPager(t *testing.T) {
	var outputSets, expectedSets []*testSet
	var outputPages, expectedPages []*testPage

	p := &SetPager{
		MaxPageSize: 4,

		OutputSet: func(_ context.Context, total int, s Set, grps []Group) error {
			ts := s.(*testSet)
			ts.Total = total
			for _, g := range grps {
				ts.Groups = append(ts.Groups, g.(*testGroup))
			}
			outputSets = append(outputSets, ts)
			return nil
		},
		OutputPage: func(_ context.Context, s Set, g Group) error {
			ts := s.(*testSet)
			outputPages = append(outputPages, &testPage{
				Index: ts.Pages,
				Group: g.(*testGroup),
			})
			ts.Pages++
			return nil
		},

		NewSet: func(h Head) Set {
			return &testSet{Head: h.(string)}
		},
		Combine: func(l, r Group) Group {
			lg, rg := l.(*testGroup), r.(*testGroup)
			if lg.Key != rg.Key {
				return nil
			}
			lg.Vals = append(lg.Vals, rg.Vals...)
			return lg
		},
		Split: func(total int, g Group) (Group, Group) {
			tg := g.(*testGroup)
			ng := &testGroup{
				Key:  tg.Key,
				Vals: tg.Vals[:total],
			}
			tg.Vals = tg.Vals[total:]
			return ng, tg
		},
		Size: func(g Group) int { return len(g.(*testGroup).Vals) },
	}

	ctx := context.Background()
	testutil.FatalOnErrT(t, "StartSet error: %v", p.StartSet(ctx, "head key"))
	testutil.FatalOnErrT(t, "AddGroup error: %v", p.AddGroup(ctx, &testGroup{
		Key:  "key1",
		Vals: []int{11, 12, 13},
	}))
	testutil.FatalOnErrT(t, "AddGroup error: %v", p.AddGroup(ctx, &testGroup{
		Key:  "key1",
		Vals: []int{14, 15, 16},
	}))
	expectedPages = append(expectedPages, &testPage{
		Index: 0,
		Group: &testGroup{
			Key:  "key1",
			Vals: []int{11, 12, 13, 14},
		},
	})

	if err := testutil.DeepEqual(expectedSets, outputSets); err != nil {
		t.Fatalf("error checking Sets: %v", err)
	}
	if err := testutil.DeepEqual(expectedPages, outputPages); err != nil {
		t.Fatalf("error checking Pages: %v", err)
	}

	testutil.FatalOnErrT(t, "AddGroup error: %v", p.AddGroup(ctx, &testGroup{
		Key:  "key2",
		Vals: []int{21},
	}))
	testutil.FatalOnErrT(t, "AddGroup error: %v", p.AddGroup(ctx, &testGroup{
		Key:  "key2",
		Vals: []int{22, 23},
	}))
	expectedPages = append(expectedPages, &testPage{
		Index: 1,
		Group: &testGroup{
			Key:  "key2",
			Vals: []int{21, 22, 23},
		},
	})

	if err := testutil.DeepEqual(expectedSets, outputSets); err != nil {
		t.Fatalf("error checking Sets: %v", err)
	}
	if err := testutil.DeepEqual(expectedPages, outputPages); err != nil {
		t.Fatalf("error checking Pages: %v", err)
	}

	testutil.FatalOnErrT(t, "StartSet error: %v", p.StartSet(ctx, "next set"))
	expectedSets = append(expectedSets, &testSet{
		Head:  "head key",
		Total: 9,
		Groups: []*testGroup{{
			Key:  "key1",
			Vals: []int{15, 16},
		}},
		Pages: len(expectedPages),
	})

	if err := testutil.DeepEqual(expectedSets, outputSets); err != nil {
		t.Fatalf("error checking Sets: %v", err)
	}
	if err := testutil.DeepEqual(expectedPages, outputPages); err != nil {
		t.Fatalf("error checking Pages: %v", err)
	}

	testutil.FatalOnErrT(t, "StartSet error: %v", p.StartSet(ctx, "final set"))
	expectedSets = append(expectedSets, &testSet{
		Head: "next set",
	})

	if err := testutil.DeepEqual(expectedSets, outputSets); err != nil {
		t.Fatalf("error checking Sets: %v", err)
	}
	if err := testutil.DeepEqual(expectedPages, outputPages); err != nil {
		t.Fatalf("error checking Pages: %v", err)
	}

	testutil.FatalOnErrT(t, "AddGroup error: %v", p.AddGroup(ctx, &testGroup{
		Key:  "key0",
		Vals: []int{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
	}))
	expectedPages = append(expectedPages, &testPage{
		Index: 0,
		Group: &testGroup{
			Key:  "key0",
			Vals: []int{1, 2, 3, 4},
		},
	})
	expectedPages = append(expectedPages, &testPage{
		Index: 1,
		Group: &testGroup{
			Key:  "key0",
			Vals: []int{5, 6, 7, 8},
		},
	})

	if err := testutil.DeepEqual(expectedSets, outputSets); err != nil {
		t.Fatalf("error checking Sets: %v", err)
	}
	if err := testutil.DeepEqual(expectedPages, outputPages); err != nil {
		t.Fatalf("error checking Pages: %v", err)
	}

	testutil.FatalOnErrT(t, "Flush error: %v", p.Flush(ctx))
	expectedSets = append(expectedSets, &testSet{
		Head:  "final set",
		Total: 12,
		Groups: []*testGroup{{
			Key:  "key0",
			Vals: []int{9, 10, 11, 12},
		}},
		Pages: 2,
	})

	if err := testutil.DeepEqual(expectedSets, outputSets); err != nil {
		t.Fatalf("error checking Sets: %v", err)
	}
	if err := testutil.DeepEqual(expectedPages, outputPages); err != nil {
		t.Fatalf("error checking Pages: %v", err)
	}
}
