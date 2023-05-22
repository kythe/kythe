/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

// Package compare implements comparisons between Kythe values.
package compare // import "kythe.io/kythe/go/util/compare"

import (
	"bytes"
	"fmt"
	"strings"

	"github.com/google/go-cmp/cmp"
	"google.golang.org/protobuf/testing/protocmp"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// An Option changes the behavior of the generic comparisons.
type Option interface{ isOption() }

// And is an Option that unions other Options.
func And(opt ...Option) Option { return and(opt) }

type and []Option

func (and) isOption() {}

// With is an Option that provides a custom comparison function for the final
// values being compared.  Only the last With Option passed to Compare will be
// honored.
type With func(a, b any) Order

func (With) isOption() {}

type reverseOrder struct{}

func (reverseOrder) isOption() {}

// Reversed returns an Option that reverses the resulting Order from a
// comparison.
func Reversed() Option { return reverseOrder{} }

// By is an Option that transforms a value before performing a comparison.  It
// should work on both sides of the comparison equivalently.
type By func(any) any

func (By) isOption() {}

// Compare returns the Order between two arbitrary values of the same type.
//
// Only the following types are currently supported:
//
//	{string, int, int32, []byte, bool}.
//
// Options may be provided to change the semantics of the comparison.  Other
// types may be compared if an appropriate By Option transforms the values into
// a supported type or a With Option is provided for the types given.
//
// Note: this function panics if a and b are different types
func Compare(a, b any, opts ...Option) (o Order) {
	var reversed bool
	defer func() {
		if reversed {
			o = o.Reverse()
		}
	}()
	var with With
	var handleOpt func(opt Option)
	handleOpt = func(opt Option) {
		switch opt := opt.(type) {
		case By:
			a = opt(a)
			b = opt(b)
		case reverseOrder:
			reversed = !reversed
		case With:
			with = opt
		case and:
			for _, nested := range opt {
				handleOpt(nested)
			}
		default:
			panic(fmt.Errorf("unhandled Option type: %T", opt))
		}
	}
	for _, opt := range opts {
		handleOpt(opt)
	}
	if with != nil {
		return with(a, b)
	}
	switch a := a.(type) {
	case bool:
		return Bools(a, b.(bool))
	case int:
		return Ints(a, b.(int))
	case int32:
		return Ints(int(a), int(b.(int32)))
	case string:
		return Strings(a, b.(string))
	case []byte:
		return Bytes(a, b.([]byte))
	default:
		panic(fmt.Errorf("unhandled Compare type: %T", a))
	}
}

// AndThen returns o if o != EQ.  Otherwise, the Order between a and b is
// determined and returned.  AndThen can be used to chain comparisons.
//
// Examples:
//
//	Compare(a, b, By(someField)).AndThen(a, b, By(someOtherField))
//
//	Entries(e1, e2).AndThen(e1.FactValue, e2.FactValue)
func (o Order) AndThen(a, b any, opts ...Option) Order {
	if o != EQ {
		return o
	}
	return Compare(a, b, opts...)
}

// Seq sequences comparisons on the same two values over different
// Options.  If len(opts) == 0, Seq merely returns Compare(a, b).
// For len(opts) > 0, Seq returns
// Compare(a, b, opts[0]).AndThen(a, b, opts[1])....AndThen(a, b, opts[len(opts)-1]).
func Seq(a, b any, opts ...Option) Order {
	if len(opts) == 0 {
		return Compare(a, b)
	}
	for _, opt := range opts {
		// Inlined, short-circuiting AndThen
		if o := Compare(a, b, opt); o != EQ {
			return o
		}
	}
	return EQ
}

// ToOrder returns LT if c < 0, EQ if c == 0, or GT if c > 0.
func ToOrder(c int) Order {
	if c < 0 {
		return LT
	} else if c > 0 {
		return GT
	}
	return EQ
}

// An Order represents an ordering relationship between values.
type Order int

// LT, EQ, and GT are the standard values for an Order.
const (
	LT Order = -1 // lhs < rhs
	EQ Order = 0  // lhs == rhs
	GT Order = 1  // lhs > rhs
)

// Reverse reverses the Order: LT -> GT, GT -> LT; EQ -> EQ.
func (o Order) Reverse() Order {
	return Order(-1 * o)
}

func (o Order) String() string {
	switch o {
	case LT:
		return "LT"
	case GT:
		return "GT"
	case EQ:
		return "EQ"
	default:
		return fmt.Sprintf("Order(%d)", o)
	}
}

// Strings returns LT if s < t, EQ if s == t, or GT if s > t.
func Strings(s, t string) Order { return Order(strings.Compare(s, t)) }

// Bytes returns LT if s < t, EQ if s == t, or GT if s > t.
func Bytes(s, t []byte) Order { return Order(bytes.Compare(s, t)) }

// Ints returns LT if a < b, EQ if a == b, or GT if a > b.
func Ints(a, b int) Order { return ToOrder(a - b) }

// Bools returns LT if !a && b, EQ if a == b, or GT if a && !b.
func Bools(a, b bool) Order {
	if a == b {
		return EQ
	} else if !a {
		return LT
	}
	return GT
}

// Options for comparing components of *spb.VName protobuf messages.
var (
	ByVNameSignature = By(func(x any) any {
		return x.(*spb.VName).GetSignature()
	})
	ByVNameCorpus = By(func(x any) any {
		return x.(*spb.VName).GetCorpus()
	})
	ByVNameRoot = By(func(x any) any {
		return x.(*spb.VName).GetRoot()
	})
	ByVNamePath = By(func(x any) any {
		return x.(*spb.VName).GetPath()
	})
	ByVNameLanguage = By(func(x any) any {
		return x.(*spb.VName).GetLanguage()
	})
)

// VNames returns LT if v1 precedes v2, EQ if v1 and v2 are equal, or GT if v1
// follows v2, in standard order.  The ordering for VNames is defined by
// lexicographic comparison of [signature, corpus, root, path, language].
func VNames(v1, v2 *spb.VName) Order {
	return Seq(v1, v2,
		ByVNameSignature, ByVNameCorpus, ByVNameRoot, ByVNamePath, ByVNameLanguage)
}

// VNamesEqual reports whether v1 and v2 are equal.
func VNamesEqual(v1, v2 *spb.VName) bool { return VNames(v1, v2) == EQ }

// Options for comparing components of *spb.Entry protobuf messages.
var (
	ByEntrySource = And(By(func(x any) any {
		return x.(*spb.Entry).GetSource()
	}), With(func(a, b any) Order { return VNames(a.(*spb.VName), b.(*spb.VName)) }))
	ByEntryEdgeKind = By(func(x any) any { return x.(*spb.Entry).GetEdgeKind() })
	ByEntryFactName = By(func(x any) any { return x.(*spb.Entry).GetFactName() })
	ByEntryTarget   = And(By(func(x any) any {
		return x.(*spb.Entry).GetTarget()
	}), With(func(a, b any) Order { return VNames(a.(*spb.VName), b.(*spb.VName)) }))
)

// Entries reports whether e1 is LT, GT, or EQ to e2 in entry order, ignoring
// fact values (if any).
//
// The ordering for entries is defined by lexicographic comparison of
// [source, edge kind, fact name, target].
func Entries(e1, e2 *spb.Entry) Order {
	return Seq(e1, e2,
		ByEntrySource, ByEntryEdgeKind, ByEntryFactName, ByEntryTarget)
}

// ByEntries is a min-heap of entries, ordered by Entries.
type ByEntries []*spb.Entry

// Implement the sort.Interface
func (s ByEntries) Len() int           { return len(s) }
func (s ByEntries) Less(i, j int) bool { return Entries(s[i], s[j]) == LT }
func (s ByEntries) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }

// Push implements part of the heap.Interface
func (s *ByEntries) Push(v any) { *s = append(*s, v.(*spb.Entry)) }

// Pop implements part of the heap.Interface
func (s *ByEntries) Pop() any {
	old := *s
	n := len(old) - 1
	out := old[n]
	*s = old[:n]
	return out
}

// ValueEntries reports whether e1 is LT, GT, or EQ to e2 in entry order,
// including fact values (if any).
func ValueEntries(e1, e2 *spb.Entry) Order {
	return Entries(e1, e2).AndThen(e1.FactValue, e2.FactValue)
}

// EntriesEqual reports whether e1 and e2 are equivalent, including their fact
// values (if any).
func EntriesEqual(e1, e2 *spb.Entry) bool { return ValueEntries(e1, e2) == EQ }

// ProtoDiff returns a human-readable report of the differences between two
// values, ensuring that any proto.Message values are compared correctly with
// proto.Equal.
//
// See github.com/google/go-cmp/cmp for more details.
func ProtoDiff(x, y any, opts ...cmp.Option) string {
	return cmp.Diff(x, y, makeProtoOpts(opts)...)
}

func makeProtoOpts(opts []cmp.Option) []cmp.Option {
	protoOpts := append([]cmp.Option{}, opts...)
	protoOpts = append(protoOpts, protocmp.Transform())
	return protoOpts
}
