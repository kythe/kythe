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

package reduce

import (
	"context"
	"io"
	"strconv"
	"testing"

	"kythe.io/kythe/go/test/testutil"

	ipb "kythe.io/kythe/proto/internal_go_proto"
)

var ctx = context.Background()

func testSplitInput(splits ...[]any) SplitInput {
	ch := make(chan ChannelSplitInputValue, len(splits))
	for _, s := range splits {
		in := make(chan ChannelInputValue, len(s))
		for _, i := range s {
			in <- ChannelInputValue{Value: i}
		}
		close(in)
		ch <- ChannelSplitInputValue{Input: ChannelInput(in)}
	}
	close(ch)
	return ChannelSplitInput(ch)
}

func testReadInput(splits SplitInput) [][]any {
	var ss [][]any
	for {
		ri, err := splits.NextSplit()
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}

		var s []any
		for {
			i, err := ri.Next()
			if err == io.EOF {
				break
			} else if err != nil {
				panic(err)
			}
			s = append(s, i)
		}
		ss = append(ss, s)
	}
	return ss
}

func TestSort(t *testing.T) {
	sorted, err := Sort(ctx, testSplitInput([]any{1, 2, 3}, []any{1, 5, 6}), Func(func(ctx context.Context, rio IO) error {
		var sum int
		for {
			i, err := rio.Next()
			if err == io.EOF {
				break
			} else if err != nil {
				return err
			}

			sum += i.(int)
		}
		val := strconv.Itoa(sum)
		return rio.Emit(ctx, &ipb.SortedKeyValue{
			Key:   val,
			Value: []byte(val),
		})
	}))
	if err != nil {
		t.Fatal(err)
	}

	expected := [][]any{
		{&ipb.SortedKeyValue{
			Key:   "12",
			Value: []byte("12"),
		}},
		{&ipb.SortedKeyValue{
			Key:   "6",
			Value: []byte("6"),
		}},
	}
	actual := testReadInput(sorted)
	if err := testutil.DeepEqual(expected, actual); err != nil {
		t.Fatal(err)
	}
}
