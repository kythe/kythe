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
	"strings"

	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/log"
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
			Link:    []*cpb.Link{{Definition: []string{kytheuri.ToString(pi.ObjectVName(obj))}}},
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
			Kind: cpb.MarkedSource_BOX,
			Child: []*cpb.MarkedSource{{
				Kind:     cpb.MarkedSource_MODIFIER,
				PreText:  "func",
				PostText: " ",
			}},
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
					Kind: cpb.MarkedSource_LOOKUP_BY_PARAM,
				}},
			})
			firstParam = 1
		}
		fn.Child = append(fn.Child, ms)

		if sig.TypeParams().Len() > 0 {
			fn.Child = append(fn.Child, &cpb.MarkedSource{
				Kind:          cpb.MarkedSource_PARAMETER_LOOKUP_BY_TPARAM,
				PreText:       "[",
				PostText:      "]",
				PostChildText: ", ",
			})
		}

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
			var hasNamedReturn bool
			for i := 0; i < res.Len(); i++ {
				if v := res.At(i); v.Name() == "" {
					rms.Child = append(rms.Child, &cpb.MarkedSource{
						PreText: typeName(v.Type()),
					})
				} else {
					hasNamedReturn = true
					rms.Child = append(rms.Child, &cpb.MarkedSource{
						PostChildText: " ",
						Child: []*cpb.MarkedSource{{
							Kind:    cpb.MarkedSource_IDENTIFIER,
							PreText: v.Name(),
							Link:    []*cpb.Link{{Definition: []string{kytheuri.ToString(pi.ObjectVName(v))}}},
						}, {
							Kind:    cpb.MarkedSource_TYPE,
							PreText: typeName(v.Type()),
						}},
					})
				}
			}
			if res.Len() > 1 || hasNamedReturn {
				// If there is more than one result type (or the return is named), parenthesize.
				rms.PreText = " ("
				rms.PostText = ")"
				rms.PostChildText = ", "
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
	case *types.Chan:
		switch t.Dir() {
		case types.SendOnly:
			return "chan<- " + typeName(t.Elem())
		case types.RecvOnly:
			return "<-chan " + typeName(t.Elem())
		default:
			return "chan " + typeName(t.Elem())
		}
	case *types.Array:
		return fmt.Sprintf("[%d]%s", t.Len(), typeName(t.Elem()))
	case *types.Map:
		return fmt.Sprintf("map[%s]%s", typeName(t.Key()), typeName(t.Elem()))
	case *types.Slice:
		return "[]" + typeName(t.Elem())
	}
	return typ.String()
}

// typeArgs returns a human readable string for a type's list of type arguments
// (or "" if the type does not have any).
func typeArgs(typ types.Type) string {
	n, ok := deref(typ).(*types.Named)
	if !ok || n.TypeArgs().Len() == 0 {
		return ""
	}
	args := n.TypeArgs()
	var ss []string
	for i := 0; i < args.Len(); i++ {
		ss = append(ss, typeName(args.At(i)))
	}
	return "[" + strings.Join(ss, ", ") + "]"
}

// typeContext returns the package, type, and function context identifiers that
// qualify the name of obj, if any are applicable. The result is empty if there
// are no appropriate qualifiers.
func (pi *PackageInfo) typeContext(obj types.Object) []*cpb.MarkedSource {
	var ms []*cpb.MarkedSource
	addID := func(s string, v *spb.VName) {
		id := &cpb.MarkedSource{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: s,
		}
		if v != nil {
			id.Link = []*cpb.Link{{Definition: []string{kytheuri.ToString(v)}}}
		}
		ms = append(ms, id)
	}
	for cur := pi.owner[obj]; cur != nil; cur = pi.owner[cur] {
		if t, ok := cur.(interface {
			Name() string
		}); ok {
			addID(t.Name(), pi.ObjectVName(cur))
		} else {
			addID(typeName(cur.Type()), pi.ObjectVName(cur))
		}
	}
	if pkg := obj.Pkg(); pkg != nil {
		addID(pi.importPath(pkg), pi.PackageVName[pkg])
	}
	for i, j := 0, len(ms)-1; i < j; {
		ms[i], ms[j] = ms[j], ms[i]
		i++
		j--
	}
	return ms
}

// rewriteMarkedSourceCorpus finds all tickets in the MarkedSource
// and its children and rewrites them to use the given corpus.
func rewriteMarkedSourceCorpus(ms *cpb.MarkedSource, corpus string) {
	for _, link := range ms.Link {
		for i, def := range link.Definition {
			v, err := kytheuri.ToVName(def)
			if err != nil {
				log.Errorf("parsing ticket %q: %v", def, err)
				continue
			}
			v.Corpus = corpus
			link.Definition[i] = kytheuri.ToString(v)
		}
	}
	for _, child := range ms.Child {
		rewriteMarkedSourceCorpus(child, corpus)
	}
}

// emitCode emits a code fact for the specified marked source message on the
// target, or logs a diagnostic.
func (e *emitter) emitCode(target *spb.VName, ms *cpb.MarkedSource) {
	if ms != nil {
		if e.opts.UseCompilationCorpusForAll {
			rewriteMarkedSourceCorpus(ms, e.pi.VName.Corpus)
		}
		bits, err := proto.Marshal(ms)
		if err != nil {
			log.Errorf("Unable to marshal marked source: %v", err)
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
			PostText: "/",
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
