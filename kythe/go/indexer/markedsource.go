/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

package indexer

import (
	"fmt"
	"go/types"
	"log"
	"strings"

	"kythe.io/kythe/go/util/schema/facts"

	"github.com/golang/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

// MarkedSource returns a MarkedSource message describing obj.
// See: http://www.kythe.io/docs/schema/marked-source.html.
func (pi *PackageInfo) MarkedSource(obj types.Object) *cpb.MarkedSource {
	ms := &cpb.MarkedSource{
		Child: []*cpb.MarkedSource{{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: objectName(obj),
		}},
	}

	// Include the package name as context, and for objects that hang off a
	// named struct or interface, a label for that type.
	//
	// For example, given
	//     package p
	//     var v int                // context is "p"
	//     type s struct { v int }  // context is "p.s"
	//     func (v) f(x int) {}
	//              ^ ^--------------- context is "p.v.f"
	//              \----------------- context is "p.v"
	//
	// The tree structure is:
	//
	//                 (box)
	//                   |
	//         (ctx)-----+-------(id)
	//           |                |
	//     +----"."----+(".")    name
	//     |           |
	//    (id) pkg    type
	//
	if ctx := pi.typeContext(obj); len(ctx) != 0 {
		ms.Child = append([]*cpb.MarkedSource{{
			Kind:              cpb.MarkedSource_CONTEXT,
			PostChildText:     ".",
			AddFinalListToken: true,
			Child:             ctx,
		}}, ms.Child...)
	}

	// Handle types with "interesting" superstructure specially.
	switch t := obj.(type) {
	case *types.Func:
		// For functions we include the parameters and return values, and for
		// methods the receiver.
		//
		// Methods:   func (R) Name(p1, ...) (r0, ...)
		// Functions: func Name(p0, ...) (r0, ...)
		fn := &cpb.MarkedSource{
			Kind:  cpb.MarkedSource_BOX,
			Child: []*cpb.MarkedSource{{PreText: "func "}},
		}
		sig := t.Type().(*types.Signature)
		firstParam := 0
		if recv := sig.Recv(); recv != nil {
			// Parenthesized receiver type, e.g. (R).
			fn.Child = append(fn.Child, &cpb.MarkedSource{
				Kind:     cpb.MarkedSource_PARAMETER,
				PreText:  "(",
				PostText: ") ",
				Child: []*cpb.MarkedSource{{
					Kind:    cpb.MarkedSource_TYPE,
					PreText: typeName(recv.Type()),
				}},
			})
			firstParam = 1
		}
		fn.Child = append(fn.Child, ms)

		// If there are no parameters, the lookup will not produce anything.
		// Ensure when this happens we still get parentheses for notational
		// purposes.
		if sig.Params().Len() == 0 {
			fn.Child = append(fn.Child, &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_PARAMETER,
				PreText: "()",
			})
		} else {
			fn.Child = append(fn.Child, &cpb.MarkedSource{
				Kind:          cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM,
				PreText:       "(",
				PostChildText: ", ",
				PostText:      ")",
				LookupIndex:   uint32(firstParam),
			})
		}
		if res := sig.Results(); res != nil && res.Len() > 0 {
			rms := &cpb.MarkedSource{Kind: cpb.MarkedSource_TYPE, PreText: " "}
			if res.Len() > 1 {
				// If there is more than one result type, parenthesize.
				rms.PreText = " ("
				rms.PostText = ")"
				rms.PostChildText = ", "
			}
			for i := 0; i < res.Len(); i++ {
				rms.Child = append(rms.Child, &cpb.MarkedSource{
					PreText: objectName(res.At(i)),
				})
			}
			fn.Child = append(fn.Child, rms)
		}
		ms = fn

	case *types.Var:
		// For variables and fields, include the type.
		repl := &cpb.MarkedSource{
			Kind:          cpb.MarkedSource_BOX,
			PostChildText: " ",
			Child: []*cpb.MarkedSource{
				ms,
				{Kind: cpb.MarkedSource_LOOKUP_BY_TYPED},
			},
		}
		ms = repl

	default:
		// TODO(fromberger): Handle other variations from go/types.
	}
	return ms
}

// objectName returns a human-readable name for obj if one can be inferred.  If
// the object has its own non-blank name, that is used; otherwise if the object
// is of a named type, that type's name is used. Otherwise the result is "_".
func objectName(obj types.Object) string {
	if name := obj.Name(); name != "" {
		return name // the object's given name
	} else if name := typeName(obj.Type()); name != "" {
		return name // the object's type's name
	}
	return "_" // not sure what to call it
}

// typeName returns a human readable name for typ.
func typeName(typ types.Type) string {
	switch t := typ.(type) {
	case *types.Named:
		return t.Obj().Name()
	case *types.Basic:
		return t.Name()
	case *types.Struct:
		return "struct {...}"
	case *types.Interface:
		return "interface {...}"
	case *types.Pointer:
		return "*" + typeName(t.Elem())
	}
	return typ.String()
}

// typeContext returns the package, type, and function context identifiers that
// qualify the name of obj, if any are applicable. The result is empty if there
// are no appropriate qualifiers.
func (pi *PackageInfo) typeContext(obj types.Object) []*cpb.MarkedSource {
	var ms []*cpb.MarkedSource
	addID := func(s string) {
		ms = append(ms, &cpb.MarkedSource{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: s,
		})
	}
	for cur := pi.owner[obj]; cur != nil; cur = pi.owner[cur] {
		if t, ok := cur.(interface {
			Name() string
		}); ok {
			addID(t.Name())
		} else {
			addID(typeName(cur.Type()))
		}
	}
	if pkg := obj.Pkg(); pkg != nil {
		addID(pi.importPath(pkg))
	}
	for i, j := 0, len(ms)-1; i < j; {
		ms[i], ms[j] = ms[j], ms[i]
		i++
		j--
	}
	return ms
}

// emitCode emits a code fact for the specified marked source message on the
// target, or logs a diagnostic.
func (e *emitter) emitCode(target *spb.VName, ms *cpb.MarkedSource) {
	if ms != nil {
		bits, err := proto.Marshal(ms)
		if err != nil {
			log.Printf("ERROR: Unable to marshal marked source: %v", err)
			return
		}
		e.writeFact(target, facts.Code, string(bits))
	}
}

func (e *emitter) emitPackageMarkedSource(pi *PackageInfo) {
	if !e.opts.emitMarkedSource() || len(pi.Files) == 0 {
		return
	}

	ipath := pi.ImportPath
	ms := &cpb.MarkedSource{
		Child: []*cpb.MarkedSource{{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: ipath,
		}},
	}
	if p := strings.LastIndex(ipath, "/"); p > 0 {
		ms.Child[0].PreText = ipath[p+1:]
		ms.Child = append([]*cpb.MarkedSource{{
			Kind: cpb.MarkedSource_CONTEXT,
			Child: []*cpb.MarkedSource{{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: ipath[:p],
			}},
			PostChildText: "/",
		}}, ms.Child...)
	}
	e.emitCode(pi.VName, ms)
}

func (e *emitter) emitBuiltinMarkedSource(b *spb.VName) {
	if e.opts.emitMarkedSource() {
		e.emitCode(b, &cpb.MarkedSource{
			Kind:    cpb.MarkedSource_TYPE,
			PreText: strings.TrimSuffix(b.Signature, "#builtin"),
		})
	}
}

var (
	sliceTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 1,
		}},
		PreText: "[]",
	}
	pointerTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 1,
		}},
		PreText: "*",
	}
	mapTAppMS = &cpb.MarkedSource{
		Kind:    cpb.MarkedSource_TYPE,
		PreText: "map",
		Child: []*cpb.MarkedSource{{
			Kind:     cpb.MarkedSource_BOX,
			PreText:  "[",
			PostText: "]",
			Child: []*cpb.MarkedSource{{
				Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
				LookupIndex: 1,
			}},
		}, {
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 2,
		}},
	}
	tupleTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:          cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM,
			LookupIndex:   1,
			PostChildText: ", ",
		}},
		PreText:  "(",
		PostText: ")",
	}
	variadicTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 1,
		}},
		PreText: "...",
	}
	chanOmniTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 1,
		}},
		PreText: "chan ",
	}
	chanSendTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 1,
		}},
		PreText: "chan<- ",
	}
	chanRecvTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 1,
		}},
		PreText: "<-chan ",
	}
	genericTAppMS = &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 0,
		}, {
			Kind:     cpb.MarkedSource_BOX,
			PreText:  "[",
			PostText: "]",
			Child: []*cpb.MarkedSource{{
				Kind:        cpb.MarkedSource_PARAMETER_LOOKUP_BY_PARAM,
				LookupIndex: 1,
			}},
			PostChildText: ", ",
		}},
	}
)

func arrayTAppMS(length int64) *cpb.MarkedSource {
	return &cpb.MarkedSource{
		Kind: cpb.MarkedSource_TYPE,
		Child: []*cpb.MarkedSource{{
			Kind:        cpb.MarkedSource_LOOKUP_BY_PARAM,
			LookupIndex: 1,
		}},
		PreText: fmt.Sprintf("[%d]", length),
	}
}

func chanTAppMS(dir types.ChanDir) *cpb.MarkedSource {
	switch dir {
	case types.SendOnly:
		return chanSendTAppMS
	case types.RecvOnly:
		return chanRecvTAppMS
	default:
		return chanOmniTAppMS
	}
}
