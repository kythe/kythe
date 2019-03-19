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
			Kind:    cpb.MarkedSource_IDENTIFIER,
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
