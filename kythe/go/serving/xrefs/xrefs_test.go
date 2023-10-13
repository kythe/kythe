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

package xrefs

import (
	"bytes"
	"context"
	"flag"
	"math"
	"math/rand"
	"reflect"
	"sort"
	"strconv"
	"testing"
	"testing/quick"

	"bitbucket.org/creachadair/stringset"
	"github.com/google/codesearch/index"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/storage/table"
	"kythe.io/kythe/go/test/testutil"
	"kythe.io/kythe/go/util/compare"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/span"

	"golang.org/x/text/encoding"
	"golang.org/x/text/encoding/unicode"
	"golang.org/x/text/transform"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"

	cpb "kythe.io/kythe/proto/common_go_proto"
	srvpb "kythe.io/kythe/proto/serving_go_proto"
	xpb "kythe.io/kythe/proto/xref_go_proto"
)

var (
	ctx = context.Background()

	nodes = []*srvpb.Node{
		{
			Ticket: "kythe://someCorpus?lang=otpl#signature",
			Fact:   makeFactList("/kythe/node/kind", "testNode"),
		}, {
			Ticket: "kythe://someCorpus#aTicketSig",
			Fact:   makeFactList("/kythe/node/kind", "testNode"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#something",
			Fact: makeFactList(
				"/kythe/node/kind", "name",
				"/some/other/fact", "value",
			),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#sig2",
			Fact:   makeFactList("/kythe/node/kind", "name"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#sig3",
			Fact:   makeFactList("/kythe/node/kind", "name"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#sig4",
			Fact:   makeFactList("/kythe/node/kind", "name"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl?path=/some/valid/path#a83md71",
			Fact: makeFactList(
				"/kythe/node/kind", "file",
				"/kythe/text", "; some file content here\nfinal line\n",
				"/kythe/text/encoding", "utf-8",
			),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#6-9",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "6",
				"/kythe/loc/end", "9",
			),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#27-33",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "27",
				"/kythe/loc/end", "33",
			),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#map",
			Fact:   makeFactList("/kythe/node/kind", "function"),
		}, {
			Ticket: "kythe://core?lang=otpl#empty?",
			Fact:   makeFactList("/kythe/node/kind", "function"),
		}, {
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "51",
				"/kythe/loc/end", "55",
			),
		}, {
			Ticket: "kythe://core?lang=otpl#cons",
			Fact: makeFactList(
				"/kythe/node/kind", "function",
				// Canary to ensure we don't patch anchor facts in non-anchor nodes
				"/kythe/loc/start", "51",
			),
		}, {
			Ticket: "kythe://c?path=/a/path",
			Fact: makeFactList(
				"/kythe/node/kind", "file",
				"/kythe/text/encoding", "utf-8",
				"/kythe/text", "some random text\nhere and  \n  there\nsome random text\nhere and  \n  there\n",
			),
		}, {
			Ticket: "kythe:?path=some/utf16/file",
			Fact: []*cpb.Fact{{
				Name:  "/kythe/text/encoding",
				Value: []byte("utf-16le"),
			}, {
				Name:  "/kythe/node/kind",
				Value: []byte("file"),
			}, {
				Name:  "/kythe/text",
				Value: encodeText(utf16LE, "これはいくつかのテキストです\n"),
			}},
		}, {
			Ticket: "kythe:?path=some/utf16/file#0-4",
			Fact: makeFactList(
				"/kythe/node/kind", "anchor",
				"/kythe/loc/start", "0",
				"/kythe/loc/end", "4",
			),
		}, {
			Ticket: "kythe:#documented",
			Fact: makeFactList(
				"/kythe/node/kind", "record",
			),
			DefinitionLocation: &srvpb.ExpandedAnchor{
				Ticket: "kythe:?path=def/location#defDoc",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   1,
						LineNumber:   1,
						ColumnOffset: 1,
					},
					End: &cpb.Point{
						ByteOffset:   4,
						LineNumber:   1,
						ColumnOffset: 4,
					},
				},
			},
		}, {
			Ticket: "kythe:#documentedBy",
			Fact: makeFactList(
				"/kythe/node/kind", "record",
			),
		}, {
			Ticket: "kythe:#childDoc",
			Fact: makeFactList(
				"/kythe/node/kind", "record",
			),
		}, {
			Ticket: "kythe:#childDocBy",
			Fact: makeFactList(
				"/kythe/node/kind", "record",
			),
		}, {
			Ticket: "kythe:#secondChildDoc",
			Fact: makeFactList(
				"/kythe/node/kind", "record",
			),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#withRelated",
			Fact:   makeFactList("/kythe/node/kind", "testNode"),
		}, {
			Ticket: "kythe://someCorpus?lang=otpl#withInfos",
			Fact:   makeFactList("/kythe/node/kind", "testNode"),
		}, {
			Ticket: "kythe:#aliasNode",
			Fact:   makeFactList("/kythe/node/kind", "talias"),
		}, {
			Ticket: "kythe:#indirect",
			Fact:   makeFactList("/kythe/node/kind", "indirect"),
		},
	}

	tbl = &testTable{
		Nodes: nodes,
		Decorations: []*srvpb.FileDecorations{
			{
				File: &srvpb.File{
					Ticket:   "kythe://someCorpus?lang=otpl?path=/some/valid/path#a83md71",
					Text:     []byte("; some file content here\nfinal line\n"),
					Encoding: "utf-8",
				},
			},
			{
				File: &srvpb.File{
					Ticket: "kythe://someCorpus?lang=otpl?path=/a/path#b7te37tn4",
					Text: []byte(`(defn map [f coll]
  (if (empty? coll)
    []
    (cons (f (first coll)) (map f (rest coll)))))
`),
					Encoding: "utf-8",
				},
				Decoration: []*srvpb.FileDecorations_Decoration{
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://c?lang=otpl?path=/a/path#6-9",
							StartOffset: 6,
							EndOffset:   9,

							BuildConfiguration: "test-build-config",
						},
						Kind:   "/kythe/edge/defines/binding",
						Target: "kythe://c?lang=otpl?path=/a/path#map",
					},
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
							StartOffset: 27,
							EndOffset:   33,
						},
						Kind:   "/kythe/refs",
						Target: "kythe://core?lang=otpl#empty?",

						SemanticScope: "kythe://c?lang=otpl?path=/a/path#map",
					},
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://c?lang=otpl?path=/a/path#51-55",
							StartOffset: 51,
							EndOffset:   55,
						},
						Kind:   "/kythe/refs",
						Target: "kythe://core?lang=otpl#cons",

						SemanticScope: "kythe://c?lang=otpl?path=/a/path#map",
					},
				},
				TargetOverride: []*srvpb.FileDecorations_Override{{
					Kind:                 srvpb.FileDecorations_Override_EXTENDS,
					Overriding:           "kythe://c?lang=otpl?path=/a/path#map",
					Overridden:           "kythe://c?lang=otpl#map",
					OverriddenDefinition: "kythe://c?lang=otpl?path=/b/path#mapDef",
					MarkedSource: &cpb.MarkedSource{
						Kind:    cpb.MarkedSource_IDENTIFIER,
						PreText: "OverrideMS",
					},
				}, {
					Kind:                 srvpb.FileDecorations_Override_EXTENDS,
					Overriding:           "kythe://c?lang=otpl?path=/a/path#map",
					Overridden:           "kythe://c?lang=otpl#map",
					OverriddenDefinition: "kythe://c?lang=otpl?path=/b/path#mapDefOtherConfig",
					MarkedSource: &cpb.MarkedSource{
						Kind:    cpb.MarkedSource_IDENTIFIER,
						PreText: "OverrideMS",
					},
				}},
				Target: getNodes("kythe://c?lang=otpl?path=/a/path#map", "kythe://core?lang=otpl#empty?", "kythe://core?lang=otpl#cons"),
				TargetDefinitions: []*srvpb.ExpandedAnchor{{
					Ticket:             "kythe://c?lang=otpl?path=/b/path#mapDef",
					BuildConfiguration: "test-build-config",
					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
					},
				}, {
					Ticket:             "kythe://c?lang=otpl?path=/b/path#mapDefOtherConfig",
					BuildConfiguration: "other-build-config",
					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
					},
				}},
				Diagnostic: []*cpb.Diagnostic{
					{Message: "Test diagnostic message"},
					{
						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   6,
								LineNumber:   1,
								ColumnOffset: 6,
							},
							End: &cpb.Point{
								ByteOffset:   9,
								LineNumber:   1,
								ColumnOffset: 9,
							},
						},
						Message: "Test diagnostic message w/ Span",
					},
				},
			},
			{
				File: &srvpb.File{
					Ticket:   "kythe://corpus?path=/some/proto.proto?root=generated",
					Text:     []byte("// Generated by /some/proto.proto"),
					Encoding: "utf-8",
				},
				GeneratedBy: []string{"kythe://corpus?path=/some/proto.proto"},
			},
			{
				File: &srvpb.File{
					Ticket:   "kythe://corpus?path=file/infos",
					Text:     []byte(`some file with infos`),
					Encoding: "utf-8",
				},
				Decoration: []*srvpb.FileDecorations_Decoration{
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://corpus?lang=lang?path=file/infos#0-4",
							StartOffset: 0,
							EndOffset:   4,
						},
						Kind:   "/kythe/edge/includes/ref",
						Target: "kythe://corpus?path=another/file",
					},
					{
						Anchor: &srvpb.RawAnchor{
							Ticket:      "kythe://corpus?lang=lang?path=file/infos#5-9",
							StartOffset: 5,
							EndOffset:   9,
						},
						Kind:             "/kythe/edge/ref",
						Target:           "kythe://corpus?path=def/file#node",
						TargetDefinition: "kythe://corpus?path=def/file#anchor",
					},
				},
				TargetDefinitions: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe://corpus?path=def/file#anchor",
					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
					},
				}},
				GeneratedBy: []string{"kythe://corpus?path=some/proto.proto"},
				FileInfo: []*srvpb.FileInfo{
					fi(cp("corpus", "", "file/infos"), "overallFileRev"),
					fi(cp("corpus", "", "some/proto.proto"), "generatedRev"),
					fi(cp("corpus", "", "another/file"), "anotherFileRev"),
					fi(cp("corpus", "", "def/file"), "defFileRev"),
				},
			},
		},

		RefSets: []*srvpb.PagedCrossReferences{{
			SourceTicket: "kythe://someCorpus?lang=otpl#signature",
			SourceNode:   getNode("kythe://someCorpus?lang=otpl#signature"),

			Group: []*srvpb.PagedCrossReferences_Group{{
				BuildConfig: "testConfig",
				Kind:        "%/kythe/edge/defines/binding",
				ScopedReference: []*srvpb.PagedCrossReferences_ScopedReference{{
					SemanticScope: "kythe://someCorpus?lang=otpl#scope",
					Scope: &srvpb.ExpandedAnchor{
						Ticket:             "kythe://c?lang=otpl?path=/a/path#scope",
						BuildConfiguration: "testConfig",
						Kind:               "/kythe/edge/defines/binding",

						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   27,
								LineNumber:   2,
								ColumnOffset: 10,
							},
							End: &cpb.Point{
								ByteOffset:   33,
								LineNumber:   3,
								ColumnOffset: 5,
							},
						},

						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset: 17,
								LineNumber: 2,
							},
							End: &cpb.Point{
								ByteOffset:   27,
								LineNumber:   2,
								ColumnOffset: 10,
							},
						},
						Snippet: "here and  ",
					},
					MarkedSource: &cpb.MarkedSource{
						Kind:    cpb.MarkedSource_IDENTIFIER,
						PreText: "Scope",
					},
					Reference: []*srvpb.ExpandedAnchor{{
						Ticket:             "kythe://c?lang=otpl?path=/a/path#27-33",
						BuildConfiguration: "testConfig",

						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   27,
								LineNumber:   2,
								ColumnOffset: 10,
							},
							End: &cpb.Point{
								ByteOffset:   33,
								LineNumber:   3,
								ColumnOffset: 5,
							},
						},

						SnippetSpan: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset: 17,
								LineNumber: 2,
							},
							End: &cpb.Point{
								ByteOffset:   27,
								LineNumber:   2,
								ColumnOffset: 10,
							},
						},
						Snippet: "here and  ",
					}},
				}},
			}},
			PageIndex: []*srvpb.PagedCrossReferences_PageIndex{{
				PageKey: "aBcDeFg",
				Kind:    "%/kythe/edge/ref",
				Count:   2,
			}},
		}, {
			SourceTicket: "kythe://someCorpus?lang=otpl#withRelated",
			SourceNode:   getNode("kythe://someCorpus?lang=otpl#withRelated"),
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "id",
			},

			Group: []*srvpb.PagedCrossReferences_Group{{
				Kind: "/kythe/edge/extends",
				RelatedNode: []*srvpb.PagedCrossReferences_RelatedNode{{
					Node: &srvpb.Node{
						Ticket: "kythe:#someRelatedNode",
					},
				}},
			}, {
				Kind: "/kythe/edge/param",
				RelatedNode: []*srvpb.PagedCrossReferences_RelatedNode{{
					Ordinal: 0,
					Node: &srvpb.Node{
						Ticket: "kythe:#someParameter0",
					},
				}, {
					Ordinal: 1,
					Node: &srvpb.Node{
						Ticket: "kythe:#someParameter1",
					},
				}},
			}},
		}, {
			SourceTicket: "kythe://someCorpus?lang=otpl#withInfos",
			SourceNode:   getNode("kythe://someCorpus?lang=otpl#withInfos"),

			Group: []*srvpb.PagedCrossReferences_Group{{
				Kind: "%/kythe/edge/ref",
				Anchor: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe://corpus?lang=otpl?path=some/file#27-33",

					Span: &cpb.Span{
						Start: &cpb.Point{
							ByteOffset:   27,
							LineNumber:   2,
							ColumnOffset: 10,
						},
						End: &cpb.Point{
							ByteOffset:   33,
							LineNumber:   3,
							ColumnOffset: 5,
						},
					},
				}},

				FileInfo: []*srvpb.FileInfo{
					fi(cp("corpus", "", "some/file"), "someFileRev"),
				},
			}},
		}, {
			SourceTicket: "kythe://someCorpus?lang=otpl#withMerge",
			SourceNode:   getNode("kythe://someCorpus?lang=otpl#withMerge"),
			MergeWith: []string{
				"kythe://someCorpus?lang=otpl#withCallers",
				"kythe://someCorpus?lang=otpl#withRelated",
			},
		}, {
			SourceTicket: "kythe://someCorpus?lang=otpl#withCallers",
			SourceNode:   getNode("kythe://someCorpus?lang=otpl#withCallers"),

			Group: []*srvpb.PagedCrossReferences_Group{{
				Kind: "#internal/ref/call/direct",
				Caller: []*srvpb.PagedCrossReferences_Caller{{
					Caller: &srvpb.ExpandedAnchor{
						Ticket: "kythe:?path=someFile#someCallerAnchor",
						Span:   arbitrarySpan,
					},
					MarkedSource: &cpb.MarkedSource{
						Kind:    cpb.MarkedSource_IDENTIFIER,
						PreText: "id",
					},
					SemanticCaller: "kythe:#someCaller",
					Callsite: []*srvpb.ExpandedAnchor{{
						Ticket: "kythe:?path=someFile#someCallsiteAnchor",
					}},
				}},
			}, {
				Kind: "#internal/ref/call/override",
				Caller: []*srvpb.PagedCrossReferences_Caller{{
					Caller: &srvpb.ExpandedAnchor{
						Ticket: "kythe:?path=someFile#someOverrideCallerAnchor1",
						Span:   arbitrarySpan,
					},
					Callsite: []*srvpb.ExpandedAnchor{{
						Ticket: "kythe:?path=someFile#someCallsiteAnchor",
						Span:   arbitrarySpan,
					}},
				}, {
					Caller: &srvpb.ExpandedAnchor{
						Ticket: "kythe:?path=someFile#someOverrideCallerAnchor2",
						Span:   arbitrarySpan,
					},
					Callsite: []*srvpb.ExpandedAnchor{{
						Ticket: "kythe:?path=someFile#someCallsiteAnchor",
					}},
				}},
			}},
		}, {
			SourceTicket: "kythe:#aliasNode",
			SourceNode:   getNode("kythe:#aliasNode"),
			Group: []*srvpb.PagedCrossReferences_Group{{
				Kind: "%/kythe/edge/aliases",
				RelatedNode: []*srvpb.PagedCrossReferences_RelatedNode{{
					Node: getNode("kythe://someCorpus?lang=otpl#signature"),
				}},
			}, {
				Kind: "/kythe/edge/indirect",
				RelatedNode: []*srvpb.PagedCrossReferences_RelatedNode{{
					Node: getNode("kythe:#indirect"),
				}},
			}, {
				Kind: "%/kythe/edge/ref",
				Anchor: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe:?path=somewhere#0-9",

					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
					},
				}},
			}},
		}, {
			SourceTicket: "kythe:#indirect",
			SourceNode:   getNode("kythe:#indirect"),
			Group: []*srvpb.PagedCrossReferences_Group{{
				Kind: "%/kythe/edge/ref",
				Anchor: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe:?path=somewhereElse#0-9",

					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
					},
				}},
			}},
			PageIndex: []*srvpb.PagedCrossReferences_PageIndex{{
				PageKey: "indirectPage",
				Kind:    "/kythe/edge/indirect",
				Count:   1,
			}},
		}},
		RefPages: []*srvpb.PagedCrossReferences_Page{{
			PageKey: "aBcDeFg",
			Group: &srvpb.PagedCrossReferences_Group{
				Kind: "%/kythe/edge/ref",
				Anchor: []*srvpb.ExpandedAnchor{{
					Ticket: "kythe:?path=some/utf16/file#0-4",

					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
					},

					SnippetSpan: &cpb.Span{
						Start: &cpb.Point{
							LineNumber: 1,
						},
						End: &cpb.Point{
							ByteOffset:   28,
							LineNumber:   1,
							ColumnOffset: 28,
						},
					},
					Snippet: "これはいくつかのテキストです",
				}, {
					Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",

					Span: &cpb.Span{
						Start: &cpb.Point{
							ByteOffset:   51,
							LineNumber:   4,
							ColumnOffset: 15,
						},
						End: &cpb.Point{
							ByteOffset:   55,
							LineNumber:   5,
							ColumnOffset: 2,
						},
					},

					SnippetSpan: &cpb.Span{
						Start: &cpb.Point{
							ByteOffset: 36,
							LineNumber: 4,
						},
						End: &cpb.Point{
							ByteOffset:   52,
							LineNumber:   4,
							ColumnOffset: 16,
						},
					},
					Snippet: "some random text",
				}},
			},
		}, {
			PageKey: "indirectPage",
			Group: &srvpb.PagedCrossReferences_Group{
				Kind: "/kythe/edge/indirect",
				RelatedNode: []*srvpb.PagedCrossReferences_RelatedNode{{
					Node: getNode("kythe://someCorpus?lang=otpl#signature"),
				}},
			},
		}},

		Documents: []*srvpb.Document{{
			Ticket:  "kythe:#documented",
			RawText: "some documentation text",
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "DocumentBuilderFactory",
			},
			Node:        getNodes("kythe:#documented"),
			ChildTicket: []string{"kythe:#childDoc", "kythe:#childDocBy"},
		}, {
			Ticket:       "kythe:#documentedBy",
			DocumentedBy: "kythe:#documented",
			RawText:      "replaced documentation text",
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "ReplacedDocumentBuilderFactory",
			},
			Node: getNodes("kythe:#documentedBy"),
		}, {
			Ticket:  "kythe:#childDoc",
			RawText: "child document text",
			Node:    getNodes("kythe:#childDoc"),
		}, {
			Ticket:       "kythe:#childDocBy",
			DocumentedBy: "kythe:#secondChildDoc",
			Node:         getNodes("kythe:#childDocBy"),
		}, {
			Ticket:  "kythe:#secondChildDoc",
			RawText: "second child document text",
			Node:    getNodes("kythe:#secondChildDoc"),
		}},
	}

	arbitrarySpan = &cpb.Span{
		Start: &cpb.Point{LineNumber: 1},
		End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
	}
)

func getNodes(ts ...string) []*srvpb.Node {
	var res []*srvpb.Node
	for _, t := range ts {
		res = append(res, getNode(t))
	}
	return res
}

func getNode(t string) *srvpb.Node {
	for _, n := range nodes {
		if n.Ticket == t {
			return n
		}
	}
	return &srvpb.Node{Ticket: t}
}

var utf16LE = unicode.UTF16(unicode.LittleEndian, unicode.IgnoreBOM)

func encodeText(e encoding.Encoding, text string) []byte {
	res, _, err := transform.Bytes(e.NewEncoder(), []byte(text))
	if err != nil {
		panic(err)
	}
	return res
}

func TestDecorationsRefs(t *testing.T) {
	d := tbl.Decorations[1]

	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: d.File.Ticket},
		References: true,
		Filter:     []string{"**"},
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	if reply.SourceText != nil {
		t.Errorf("Unexpected source text: %q", string(reply.SourceText))
	}
	if reply.Encoding != "" {
		t.Errorf("Unexpected encoding: %q", reply.Encoding)
	}

	expected := refs(span.NewNormalizer(d.File.Text), d.Decoration, d.FileInfo)
	for _, ref := range expected {
		ref.SemanticScope = "" // not requested
	}
	if err := testutil.DeepEqual(expected, reply.Reference); err != nil {
		t.Fatal(err)
	}

	expectedNodes := nodeInfos(tbl.Nodes[9:11], tbl.Nodes[12:13])
	if err := testutil.DeepEqual(expectedNodes, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestDecorationsRefScopes(t *testing.T) {
	d := tbl.Decorations[1]

	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:       &xpb.Location{Ticket: d.File.Ticket},
		References:     true,
		SemanticScopes: true,
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	expected := refs(span.NewNormalizer(d.File.Text), d.Decoration, d.FileInfo)
	if err := testutil.DeepEqual(expected, reply.Reference); err != nil {
		t.Fatal(err)
	}
}

func TestDecorationsExtendsOverrides(t *testing.T) {
	d := tbl.Decorations[1]

	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:          &xpb.Location{Ticket: d.File.Ticket},
		References:        true,
		ExtendsOverrides:  true,
		SemanticScopes:    true,
		TargetDefinitions: true,
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	expectedOverrides := map[string]*xpb.DecorationsReply_Overrides{
		"kythe://c?lang=otpl?path=/a/path#map": &xpb.DecorationsReply_Overrides{
			Override: []*xpb.DecorationsReply_Override{{
				Kind:             xpb.DecorationsReply_Override_EXTENDS,
				Target:           "kythe://c?lang=otpl#map",
				TargetDefinition: "kythe://c?lang=otpl?path=/b/path#mapDef",
				MarkedSource: &cpb.MarkedSource{
					Kind:    cpb.MarkedSource_IDENTIFIER,
					PreText: "OverrideMS",
				},
			}, {
				Kind:             xpb.DecorationsReply_Override_EXTENDS,
				Target:           "kythe://c?lang=otpl#map",
				TargetDefinition: "kythe://c?lang=otpl?path=/b/path#mapDefOtherConfig",
				MarkedSource: &cpb.MarkedSource{
					Kind:    cpb.MarkedSource_IDENTIFIER,
					PreText: "OverrideMS",
				},
			}},
		},
	}
	if err := testutil.DeepEqual(expectedOverrides, reply.ExtendsOverrides); err != nil {
		t.Fatal(err)
	}

	expectedDefs := map[string]*xpb.Anchor{
		"kythe://c?lang=otpl?path=/b/path#mapDef": &xpb.Anchor{
			Ticket:      "kythe://c?lang=otpl?path=/b/path#mapDef",
			Parent:      "kythe://c?path=/b/path",
			BuildConfig: "test-build-config",
			Span: &cpb.Span{
				Start: &cpb.Point{LineNumber: 1},
				End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
			},
		},
		"kythe://c?lang=otpl?path=/b/path#mapDefOtherConfig": &xpb.Anchor{
			Ticket:      "kythe://c?lang=otpl?path=/b/path#mapDefOtherConfig",
			Parent:      "kythe://c?path=/b/path",
			BuildConfig: "other-build-config",
			Span: &cpb.Span{
				Start: &cpb.Point{LineNumber: 1},
				End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
			},
		},
	}
	if err := testutil.DeepEqual(expectedDefs, reply.DefinitionLocations); err != nil {
		t.Fatal(err)
	}
}

func TestDecorationsBuildConfig(t *testing.T) {
	d := tbl.Decorations[1]
	st := tbl.Construct(t)

	t.Run("MissingConfig", func(t *testing.T) {
		reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
			Location:          &xpb.Location{Ticket: d.File.Ticket},
			References:        true,
			BuildConfig:       []string{"missing-build-config"},
			ExtendsOverrides:  true,
			TargetDefinitions: true,
		})
		testutil.Fatalf(t, "DecorationsRequest error: %v", err)

		if err := testutil.DeepEqual([]*xpb.DecorationsReply_Reference{}, reply.Reference); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("FoundConfig", func(t *testing.T) {
		reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
			Location:          &xpb.Location{Ticket: d.File.Ticket},
			References:        true,
			BuildConfig:       []string{"test-build-config"},
			ExtendsOverrides:  true,
			TargetDefinitions: true,
		})
		testutil.Fatalf(t, "DecorationsRequest error: %v", err)

		expected := refs(span.NewNormalizer(d.File.Text), d.Decoration[:1], d.FileInfo)
		if err := testutil.DeepEqual(expected, reply.Reference); err != nil {
			t.Fatal(err)
		}

		expectedOverrides := map[string]*xpb.DecorationsReply_Overrides{
			"kythe://c?lang=otpl?path=/a/path#map": &xpb.DecorationsReply_Overrides{
				Override: []*xpb.DecorationsReply_Override{{
					Kind:             xpb.DecorationsReply_Override_EXTENDS,
					Target:           "kythe://c?lang=otpl#map",
					TargetDefinition: "kythe://c?lang=otpl?path=/b/path#mapDef",
					MarkedSource: &cpb.MarkedSource{
						Kind:    cpb.MarkedSource_IDENTIFIER,
						PreText: "OverrideMS",
					},
				}},
			},
		}
		if err := testutil.DeepEqual(expectedOverrides, reply.ExtendsOverrides); err != nil {
			t.Fatal(err)
		}

		expectedDefs := map[string]*xpb.Anchor{
			"kythe://c?lang=otpl?path=/b/path#mapDef": &xpb.Anchor{
				Ticket:      "kythe://c?lang=otpl?path=/b/path#mapDef",
				Parent:      "kythe://c?path=/b/path",
				BuildConfig: "test-build-config",
				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
				},
			},
		}
		if err := testutil.DeepEqual(expectedDefs, reply.DefinitionLocations); err != nil {
			t.Fatal(err)
		}
	})
}

func TestDecorationsDirtyBuffer(t *testing.T) {
	d := tbl.Decorations[1]

	st := tbl.Construct(t)
	// s/empty?/seq/
	dirty := []byte(`(defn map [f coll]
  (if (seq coll)
    []
    (cons (f (first coll)) (map f (rest coll)))))
`)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:    &xpb.Location{Ticket: d.File.Ticket},
		DirtyBuffer: dirty,
		References:  true,
		Filter:      []string{"**"},
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	if reply.SourceText != nil {
		t.Errorf("Unexpected source text: %q", string(reply.SourceText))
	}
	if reply.Encoding != "" {
		t.Errorf("Unexpected encoding: %q", reply.Encoding)
	}

	expected := []*xpb.DecorationsReply_Reference{
		{
			// Unpatched anchor for "map"
			TargetTicket: "kythe://c?lang=otpl?path=/a/path#map",
			Kind:         "/kythe/edge/defines/binding",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   6,
					LineNumber:   1,
					ColumnOffset: 6,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					LineNumber:   1,
					ColumnOffset: 9,
				},
			},

			BuildConfig: "test-build-config",
		},
		// Skipped anchor for "empty?" (inside edit region)
		{
			// Patched anchor for "cons" (moved backwards by 3 bytes)
			TargetTicket: "kythe://core?lang=otpl#cons",
			Kind:         "/kythe/refs",
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   48,
					LineNumber:   4,
					ColumnOffset: 5,
				},
				End: &cpb.Point{
					ByteOffset:   52,
					LineNumber:   4,
					ColumnOffset: 9,
				},
			},
		},
	}
	if err := testutil.DeepEqual(expected, reply.Reference); err != nil {
		t.Fatal(err)
	}

	// These are a subset of the anchor nodes in tbl.Decorations[1].  tbl.Nodes[10] is missing because
	// it is the target of an anchor in the edited region.
	expectedNodes := nodeInfos([]*srvpb.Node{tbl.Nodes[9], tbl.Nodes[12]})
	if err := testutil.DeepEqual(expectedNodes, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestDecorationsNotFound(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: "kythe:#someMissingFileTicket",
		},
	})

	if err == nil {
		t.Fatalf("Unexpected DecorationsReply: {%v}", reply)
	} else if err != xrefs.ErrDecorationsNotFound {
		t.Fatalf("Unexpected Decorations error: %v", err)
	}
}

func TestDecorationsEmpty(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location: &xpb.Location{
			Ticket: tbl.Decorations[0].File.Ticket,
		},
		References: true,
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	if len(reply.Reference) > 0 {
		t.Fatalf("Unexpected DecorationsReply: {%v}", reply)
	}
}

func TestDecorationsSourceText(t *testing.T) {
	expected := tbl.Decorations[0]

	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:   &xpb.Location{Ticket: expected.File.Ticket},
		SourceText: true,
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	if !bytes.Equal(reply.SourceText, expected.File.Text) {
		t.Errorf("Expected source text %q; found %q", string(expected.File.Text), string(reply.SourceText))
	}
	if reply.Encoding != expected.File.Encoding {
		t.Errorf("Expected source text %q; found %q", expected.File.Encoding, reply.Encoding)
	}
	if len(reply.Reference) > 0 {
		t.Errorf("Unexpected references in DecorationsReply %v", reply.Reference)
	}
}

func TestDecorationsGeneratedBy(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location: &xpb.Location{Ticket: "kythe://corpus?path=/some/proto.proto?root=generated"},
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	expected := &xpb.DecorationsReply{
		Location: &xpb.Location{Ticket: "kythe://corpus?path=/some/proto.proto?root=generated"},
		GeneratedByFile: []*xpb.File{{
			CorpusPath: &cpb.CorpusPath{
				Corpus: "corpus",
				Path:   "/some/proto.proto",
			},
		}},
	}

	if diff := compare.ProtoDiff(expected, reply); diff != "" {
		t.Fatalf("Unexpected diff (- expected; + found):\n%s", diff)
	}
}

func TestDecorationsRevisions(t *testing.T) {
	st := tbl.Construct(t)

	t.Run("file/generated_by", func(t *testing.T) {
		reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
			Location: &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
		})
		testutil.Fatalf(t, "DecorationsRequest error: %v", err)

		expected := &xpb.DecorationsReply{
			Location: &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
			Revision: "overallFileRev",
			GeneratedByFile: []*xpb.File{{
				CorpusPath: &cpb.CorpusPath{
					Corpus: "corpus",
					Path:   "some/proto.proto",
				},
				Revision: "generatedRev",
			}},
		}

		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("Unexpected diff (- expected; + found):\n%s", diff)
		}
	})

	t.Run("target_revision", func(t *testing.T) {
		reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
			Location:   &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
			References: true,
		})
		testutil.Fatalf(t, "DecorationsRequest error: %v", err)

		expected := &xpb.DecorationsReply{
			Location: &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
			Revision: "overallFileRev",
			GeneratedByFile: []*xpb.File{{
				CorpusPath: &cpb.CorpusPath{
					Corpus: "corpus",
					Path:   "some/proto.proto",
				},
				Revision: "generatedRev",
			}},
			Reference: []*xpb.DecorationsReply_Reference{{
				TargetTicket:   "kythe://corpus?path=another/file",
				Kind:           "/kythe/edge/includes/ref",
				TargetRevision: "anotherFileRev",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   0,
						LineNumber:   1,
						ColumnOffset: 0,
					},
					End: &cpb.Point{
						ByteOffset:   4,
						LineNumber:   1,
						ColumnOffset: 4,
					},
				},
			}, {
				TargetTicket: "kythe://corpus?path=def/file#node",
				Kind:         "/kythe/edge/ref",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   5,
						LineNumber:   1,
						ColumnOffset: 5,
					},
					End: &cpb.Point{
						ByteOffset:   9,
						LineNumber:   1,
						ColumnOffset: 9,
					},
				},
			}},
		}

		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("Unexpected diff (- expected; + found):\n%s", diff)
		}
	})

	t.Run("target_defs", func(t *testing.T) {
		reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
			Location:          &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
			References:        true,
			TargetDefinitions: true,
		})
		testutil.Fatalf(t, "DecorationsRequest error: %v", err)

		expected := &xpb.DecorationsReply{
			Location: &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
			Revision: "overallFileRev",
			GeneratedByFile: []*xpb.File{{
				CorpusPath: &cpb.CorpusPath{
					Corpus: "corpus",
					Path:   "some/proto.proto",
				},
				Revision: "generatedRev",
			}},
			Reference: []*xpb.DecorationsReply_Reference{{
				TargetTicket:   "kythe://corpus?path=another/file",
				Kind:           "/kythe/edge/includes/ref",
				TargetRevision: "anotherFileRev",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   0,
						LineNumber:   1,
						ColumnOffset: 0,
					},
					End: &cpb.Point{
						ByteOffset:   4,
						LineNumber:   1,
						ColumnOffset: 4,
					},
				},
			}, {
				TargetTicket:     "kythe://corpus?path=def/file#node",
				TargetDefinition: "kythe://corpus?path=def/file#anchor",
				Kind:             "/kythe/edge/ref",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   5,
						LineNumber:   1,
						ColumnOffset: 5,
					},
					End: &cpb.Point{
						ByteOffset:   9,
						LineNumber:   1,
						ColumnOffset: 9,
					},
				},
			}},
			DefinitionLocations: map[string]*xpb.Anchor{
				"kythe://corpus?path=def/file#anchor": &xpb.Anchor{
					Ticket:   "kythe://corpus?path=def/file#anchor",
					Parent:   "kythe://corpus?path=def/file",
					Revision: "defFileRev",
					Span: &cpb.Span{
						Start: &cpb.Point{LineNumber: 1},
						End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
					},
				},
			},
		}

		if diff := compare.ProtoDiff(expected, reply); diff != "" {
			t.Fatalf("Unexpected diff (- expected; + found):\n%s", diff)
		}
	})
}

func TestDecorationsDiagnostics(t *testing.T) {
	d := tbl.Decorations[1]

	st := tbl.Construct(t)
	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:    &xpb.Location{Ticket: d.File.Ticket},
		Diagnostics: true,
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	expected := tbl.Decorations[1].Diagnostic
	if err := testutil.DeepEqual(expected, reply.Diagnostic); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesNone(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{"kythe://someCorpus?lang=otpl#sig2"},
		DefinitionKind: xpb.CrossReferencesRequest_ALL_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	if len(reply.CrossReferences) > 0 || len(reply.Nodes) > 0 {
		t.Fatalf("Expected empty CrossReferencesReply; found %v", reply)
	}
}

func TestCrossReferences(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket: "kythe:?path=some/utf16/file#0-4",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe:?path=some/utf16/file",

			Span: &cpb.Span{
				Start: &cpb.Point{LineNumber: 1},
				End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   28,
					LineNumber:   1,
					ColumnOffset: 28,
				},
			},
			Snippet: "これはいくつかのテキストです",
		}}, {Anchor: &xpb.Anchor{
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe://c?path=/a/path",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   51,
					LineNumber:   4,
					ColumnOffset: 15,
				},
				End: &cpb.Point{
					ByteOffset:   55,
					LineNumber:   5,
					ColumnOffset: 2,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 36,
					LineNumber: 4,
				},
				End: &cpb.Point{
					ByteOffset:   52,
					LineNumber:   4,
					ColumnOffset: 16,
				},
			},
			Snippet: "some random text",
		}}},

		Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
			Kind:        "/kythe/edge/defines/binding",
			Parent:      "kythe://c?path=/a/path",
			BuildConfig: "testConfig",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
				End: &cpb.Point{
					ByteOffset:   33,
					LineNumber:   3,
					ColumnOffset: 5,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 17,
					LineNumber: 2,
				},
				End: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
			},
			Snippet: "here and  ",
		}}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Definitions: 1,
		References:  2,
		RefEdgeToCount: map[string]int64{
			"/kythe/edge/ref": 2,
		},
	}, reply.Total); err != nil {
		t.Error(err)
	}
	wantFiltered := &xpb.CrossReferencesReply_Total{}
	if err := testutil.DeepEqual(wantFiltered, reply.Filtered); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	}
	sort.Sort(byOffset(xr.Reference))

	if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesScoped(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,
		SemanticScopes: true,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket: "kythe:?path=some/utf16/file#0-4",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe:?path=some/utf16/file",

			Span: &cpb.Span{
				Start: &cpb.Point{LineNumber: 1},
				End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   28,
					LineNumber:   1,
					ColumnOffset: 28,
				},
			},
			Snippet: "これはいくつかのテキストです",
		}}, {Anchor: &xpb.Anchor{
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe://c?path=/a/path",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   51,
					LineNumber:   4,
					ColumnOffset: 15,
				},
				End: &cpb.Point{
					ByteOffset:   55,
					LineNumber:   5,
					ColumnOffset: 2,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 36,
					LineNumber: 4,
				},
				End: &cpb.Point{
					ByteOffset:   52,
					LineNumber:   4,
					ColumnOffset: 16,
				},
			},
			Snippet: "some random text",
		}}},

		Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{
			Ticket: "kythe://someCorpus?lang=otpl#scope",
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "Scope",
			},
			Anchor: &xpb.Anchor{
				Ticket:      "kythe://c?lang=otpl?path=/a/path#scope",
				Kind:        "/kythe/edge/defines/binding",
				Parent:      "kythe://c?path=/a/path",
				BuildConfig: "testConfig",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
					End: &cpb.Point{
						ByteOffset:   33,
						LineNumber:   3,
						ColumnOffset: 5,
					},
				},

				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset: 17,
						LineNumber: 2,
					},
					End: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
				},
				Snippet: "here and  ",
			},
			Site: []*xpb.Anchor{{
				Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
				Kind:        "/kythe/edge/defines/binding",
				Parent:      "kythe://c?path=/a/path",
				BuildConfig: "testConfig",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
					End: &cpb.Point{
						ByteOffset:   33,
						LineNumber:   3,
						ColumnOffset: 5,
					},
				},

				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset: 17,
						LineNumber: 2,
					},
					End: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
				},
				Snippet: "here and  ",
			}},
		}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Definitions: 1,
		References:  2,
		RefEdgeToCount: map[string]int64{
			"/kythe/edge/ref": 2,
		},
	}, reply.Total); err != nil {
		t.Error(err)
	}
	wantFiltered := &xpb.CrossReferencesReply_Total{}
	if err := testutil.DeepEqual(wantFiltered, reply.Filtered); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	}
	sort.Sort(byOffset(xr.Reference))

	if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesPaging(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	req := &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,
		SemanticScopes: true,
		PageSize:       1,
	}

	var anchors []*xpb.CrossReferencesReply_RelatedAnchor
	for {
		reply, err := st.CrossReferences(ctx, req)
		testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)
		xr := reply.GetCrossReferences()[ticket]
		anchors = append(anchors, xr.GetReference()...)
		anchors = append(anchors, xr.GetDefinition()...)
		anchors = append(anchors, xr.GetDeclaration()...)
		anchors = append(anchors, xr.GetCaller()...)

		req.PageToken = reply.GetNextPageToken()
		if req.PageToken == "" {
			break
		}
		t.Logf("NextPageToken: %s", req.PageToken)
	}

	expected := []*xpb.CrossReferencesReply_RelatedAnchor{
		{
			Ticket: "kythe://someCorpus?lang=otpl#scope",
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "Scope",
			},
			Anchor: &xpb.Anchor{
				Ticket:      "kythe://c?lang=otpl?path=/a/path#scope",
				Kind:        "/kythe/edge/defines/binding",
				Parent:      "kythe://c?path=/a/path",
				BuildConfig: "testConfig",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
					End: &cpb.Point{
						ByteOffset:   33,
						LineNumber:   3,
						ColumnOffset: 5,
					},
				},

				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset: 17,
						LineNumber: 2,
					},
					End: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
				},
				Snippet: "here and  ",
			},
			Site: []*xpb.Anchor{{
				Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
				Kind:        "/kythe/edge/defines/binding",
				Parent:      "kythe://c?path=/a/path",
				BuildConfig: "testConfig",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
					End: &cpb.Point{
						ByteOffset:   33,
						LineNumber:   3,
						ColumnOffset: 5,
					},
				},

				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset: 17,
						LineNumber: 2,
					},
					End: &cpb.Point{
						ByteOffset:   27,
						LineNumber:   2,
						ColumnOffset: 10,
					},
				},
				Snippet: "here and  ",
			}},
		},
		{
			Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=some/utf16/file#0-4",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=some/utf16/file",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
				},

				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						LineNumber: 1,
					},
					End: &cpb.Point{
						ByteOffset:   28,
						LineNumber:   1,
						ColumnOffset: 28,
					},
				},
				Snippet: "これはいくつかのテキストです",
			},
		},
		{
			Anchor: &xpb.Anchor{
				Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe://c?path=/a/path",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   51,
						LineNumber:   4,
						ColumnOffset: 15,
					},
					End: &cpb.Point{
						ByteOffset:   55,
						LineNumber:   5,
						ColumnOffset: 2,
					},
				},

				SnippetSpan: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset: 36,
						LineNumber: 4,
					},
					End: &cpb.Point{
						ByteOffset:   52,
						LineNumber:   4,
						ColumnOffset: 16,
					},
				},
				Snippet: "some random text",
			},
		},
	}

	if err := testutil.DeepEqual(expected, anchors); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesReadAhead(t *testing.T) {
	const flagName = "page_read_ahead"
	val := flag.Lookup(flagName).Value.String()
	testutil.Fatalf(t, "flag.Set: %v", flag.Set(flagName, "4"))
	defer flag.Set(flagName, val)

	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket: "kythe:?path=some/utf16/file#0-4",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe:?path=some/utf16/file",

			Span: &cpb.Span{
				Start: &cpb.Point{LineNumber: 1},
				End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					LineNumber: 1,
				},
				End: &cpb.Point{
					ByteOffset:   28,
					LineNumber:   1,
					ColumnOffset: 28,
				},
			},
			Snippet: "これはいくつかのテキストです",
		}}, {Anchor: &xpb.Anchor{
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe://c?path=/a/path",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   51,
					LineNumber:   4,
					ColumnOffset: 15,
				},
				End: &cpb.Point{
					ByteOffset:   55,
					LineNumber:   5,
					ColumnOffset: 2,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 36,
					LineNumber: 4,
				},
				End: &cpb.Point{
					ByteOffset:   52,
					LineNumber:   4,
					ColumnOffset: 16,
				},
			},
			Snippet: "some random text",
		}}},

		Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
			Kind:        "/kythe/edge/defines/binding",
			Parent:      "kythe://c?path=/a/path",
			BuildConfig: "testConfig",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
				End: &cpb.Point{
					ByteOffset:   33,
					LineNumber:   3,
					ColumnOffset: 5,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 17,
					LineNumber: 2,
				},
				End: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
			},
			Snippet: "here and  ",
		}}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Definitions: 1,
		References:  2,
		RefEdgeToCount: map[string]int64{
			"/kythe/edge/ref": 2,
		},
	}, reply.Total); err != nil {
		t.Error(err)
	}
	wantFiltered := &xpb.CrossReferencesReply_Total{}
	if err := testutil.DeepEqual(wantFiltered, reply.Filtered); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	}
	sort.Sort(byOffset(xr.Reference))

	if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

type mockPatcher struct {
	files []*srvpb.FileInfo
}

func (p *mockPatcher) Close() error { return nil }
func (p *mockPatcher) AddFile(ctx context.Context, f *srvpb.FileInfo) error {
	if f != nil {
		p.files = append(p.files, f)
	}
	return nil
}
func (p *mockPatcher) patchSpan(span *cpb.Span) {
	if span == nil {
		return
	}
	// Just move everything over by 1-ish
	span.Start.ByteOffset++
	span.Start.LineNumber++
	span.Start.ColumnOffset++
	span.End.ByteOffset++
	span.End.LineNumber++
	span.End.ColumnOffset++
}
func (p *mockPatcher) patchAnchor(a *xpb.Anchor) {
	p.patchSpan(a.Span)
	p.patchSpan(a.SnippetSpan)
}

func (p *mockPatcher) PatchAnchors(ctx context.Context, as []*xpb.Anchor) ([]*xpb.Anchor, error) {
	for _, a := range as {
		p.patchAnchor(a)
	}
	return as, nil
}
func (p *mockPatcher) PatchRelatedAnchors(ctx context.Context, as []*xpb.CrossReferencesReply_RelatedAnchor) ([]*xpb.CrossReferencesReply_RelatedAnchor, error) {
	for _, a := range as {
		p.patchAnchor(a.Anchor)
		for _, site := range a.Site {
			p.patchAnchor(site)
		}
	}
	return as, nil
}

func TestDecorationsPatching(t *testing.T) {
	st := tbl.Construct(t)

	patcher := &mockPatcher{}
	st.MakePatcher = func(ctx context.Context, ws *xpb.Workspace) (MultiFilePatcher, error) {
		return patcher, nil
	}

	reply, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:          &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
		References:        true,
		TargetDefinitions: true,

		Workspace:             &xpb.Workspace{Uri: "test:"},
		PatchAgainstWorkspace: true,
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	expected := &xpb.DecorationsReply{
		Location: &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
		Revision: "overallFileRev",
		GeneratedByFile: []*xpb.File{{
			CorpusPath: &cpb.CorpusPath{
				Corpus: "corpus",
				Path:   "some/proto.proto",
			},
			Revision: "generatedRev",
		}},
		Reference: []*xpb.DecorationsReply_Reference{{
			TargetTicket:   "kythe://corpus?path=another/file",
			Kind:           "/kythe/edge/includes/ref",
			TargetRevision: "anotherFileRev",
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   0,
					LineNumber:   1,
					ColumnOffset: 0,
				},
				End: &cpb.Point{
					ByteOffset:   4,
					LineNumber:   1,
					ColumnOffset: 4,
				},
			},
		}, {
			TargetTicket:     "kythe://corpus?path=def/file#node",
			TargetDefinition: "kythe://corpus?path=def/file#anchor",
			Kind:             "/kythe/edge/ref",
			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   5,
					LineNumber:   1,
					ColumnOffset: 5,
				},
				End: &cpb.Point{
					ByteOffset:   9,
					LineNumber:   1,
					ColumnOffset: 9,
				},
			},
		}},
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe://corpus?path=def/file#anchor": &xpb.Anchor{
				Ticket:   "kythe://corpus?path=def/file#anchor",
				Parent:   "kythe://corpus?path=def/file",
				Revision: "defFileRev",
				Span: &cpb.Span{
					Start: &cpb.Point{ByteOffset: 1, LineNumber: 2, ColumnOffset: 1},
					End:   &cpb.Point{ByteOffset: 5, LineNumber: 2, ColumnOffset: 5},
				},
			},
		},
	}

	if diff := compare.ProtoDiff(expected, reply); diff != "" {
		t.Fatalf("Unexpected diff (- expected; + found):\n%s", diff)
	}
}

func TestPatchingError(t *testing.T) {
	st := tbl.Construct(t)

	// An error when creating a patcher should not result in an overall error.
	st.MakePatcher = func(ctx context.Context, ws *xpb.Workspace) (MultiFilePatcher, error) {
		return nil, context.Canceled
	}

	_, err := st.Decorations(ctx, &xpb.DecorationsRequest{
		Location:          &xpb.Location{Ticket: "kythe://corpus?path=file/infos"},
		References:        true,
		TargetDefinitions: true,

		Workspace:             &xpb.Workspace{Uri: "test:"},
		PatchAgainstWorkspace: true,
	})
	testutil.Fatalf(t, "DecorationsRequest error: %v", err)

	_, err = st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{"kythe://someCorpus?lang=otpl#signature"},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,

		Workspace:             &xpb.Workspace{Uri: "test:"},
		PatchAgainstWorkspace: true,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	_, err = st.Documentation(ctx, &xpb.DocumentationRequest{
		Ticket: []string{"kythe:#documented"},

		IncludeChildren: true,

		Workspace:             &xpb.Workspace{Uri: "test:"},
		PatchAgainstWorkspace: true,
	})
	testutil.Fatalf(t, "DocumentationRequest error: %v", err)
}

func TestCrossReferencesPatching(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	patcher := &mockPatcher{}
	st.MakePatcher = func(ctx context.Context, ws *xpb.Workspace) (MultiFilePatcher, error) {
		return patcher, nil
	}
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,

		Workspace:             &xpb.Workspace{Uri: "test:"},
		PatchAgainstWorkspace: true,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket: "kythe:?path=some/utf16/file#0-4",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe:?path=some/utf16/file",

			Span: &cpb.Span{
				Start: &cpb.Point{ByteOffset: 1, LineNumber: 2, ColumnOffset: 1},
				End:   &cpb.Point{ByteOffset: 5, LineNumber: 2, ColumnOffset: 5},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   1,
					LineNumber:   2,
					ColumnOffset: 1,
				},
				End: &cpb.Point{
					ByteOffset:   29,
					LineNumber:   2,
					ColumnOffset: 29,
				},
			},
			Snippet: "これはいくつかのテキストです",
		}}, {Anchor: &xpb.Anchor{
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe://c?path=/a/path",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   52,
					LineNumber:   5,
					ColumnOffset: 16,
				},
				End: &cpb.Point{
					ByteOffset:   56,
					LineNumber:   6,
					ColumnOffset: 3,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   37,
					LineNumber:   5,
					ColumnOffset: 1,
				},
				End: &cpb.Point{
					ByteOffset:   53,
					LineNumber:   5,
					ColumnOffset: 17,
				},
			},
			Snippet: "some random text",
		}}},

		Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
			Kind:        "/kythe/edge/defines/binding",
			Parent:      "kythe://c?path=/a/path",
			BuildConfig: "testConfig",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   28,
					LineNumber:   3,
					ColumnOffset: 11,
				},
				End: &cpb.Point{
					ByteOffset:   34,
					LineNumber:   4,
					ColumnOffset: 6,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   18,
					LineNumber:   3,
					ColumnOffset: 1,
				},
				End: &cpb.Point{
					ByteOffset:   28,
					LineNumber:   3,
					ColumnOffset: 11,
				},
			},
			Snippet: "here and  ",
		}}},
	}
	var expectedInfos []*srvpb.FileInfo

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	}
	sort.Sort(byOffset(xr.Reference))

	if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
	if err := testutil.DeepEqual(expectedInfos, patcher.files); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesFiltering(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_BINDING_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,

		CorpusPathFilters: mustParseFilters(`
filter: {
  type: INCLUDE_ONLY
	corpus: "^c$"
}
		`),
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
			Kind:   "/kythe/edge/ref",
			Parent: "kythe://c?path=/a/path",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   51,
					LineNumber:   4,
					ColumnOffset: 15,
				},
				End: &cpb.Point{
					ByteOffset:   55,
					LineNumber:   5,
					ColumnOffset: 2,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 36,
					LineNumber: 4,
				},
				End: &cpb.Point{
					ByteOffset:   52,
					LineNumber:   4,
					ColumnOffset: 16,
				},
			},
			Snippet: "some random text",
		}}},

		Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
			Kind:        "/kythe/edge/defines/binding",
			Parent:      "kythe://c?path=/a/path",
			BuildConfig: "testConfig",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
				End: &cpb.Point{
					ByteOffset:   33,
					LineNumber:   3,
					ColumnOffset: 5,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 17,
					LineNumber: 2,
				},
				End: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
			},
			Snippet: "here and  ",
		}}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Definitions: 1,
		References:  1,
		RefEdgeToCount: map[string]int64{
			"/kythe/edge/ref": 1,
		},
	}, reply.Total); err != nil {
		t.Error(err)
	}
	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		References: 1,
		RefEdgeToCount: map[string]int64{
			"/kythe/edge/ref": 1,
		},
	}, reply.Filtered); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %s", reply)
	}
	sort.Sort(byOffset(xr.Reference))

	if diff := compare.ProtoDiff(expected, xr); diff != "" {
		t.Fatalf("(-expected; +found):\n%s", diff)
	}
}

func TestCrossReferences_BuildConfigRefs(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#signature"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_ALL_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_DEFAULT,
		BuildConfig:    []string{"testConfig"},
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Definition: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
			Ticket:      "kythe://c?lang=otpl?path=/a/path#27-33",
			Kind:        "/kythe/edge/defines/binding",
			Parent:      "kythe://c?path=/a/path",
			BuildConfig: "testConfig",

			Span: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
				End: &cpb.Point{
					ByteOffset:   33,
					LineNumber:   3,
					ColumnOffset: 5,
				},
			},

			SnippetSpan: &cpb.Span{
				Start: &cpb.Point{
					ByteOffset: 17,
					LineNumber: 2,
				},
				End: &cpb.Point{
					ByteOffset:   27,
					LineNumber:   2,
					ColumnOffset: 10,
				},
			},
			Snippet: "here and  ",
		}}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Definitions: 1,
	}, reply.Total); err != nil {
		t.Error(err)
	}
	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{}, reply.Filtered); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	}
	sort.Sort(byOffset(xr.Reference))

	if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesRelatedNodes(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#withRelated"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket: []string{ticket},
		Filter: []string{"**"},
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,
		MarkedSource: &cpb.MarkedSource{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: "id",
		},

		RelatedNode: []*xpb.CrossReferencesReply_RelatedNode{{
			Ticket:       "kythe:#someRelatedNode",
			RelationKind: "/kythe/edge/extends",
		}, {
			Ticket:       "kythe:#someParameter0",
			RelationKind: "/kythe/edge/param",
			Ordinal:      0,
		}, {
			Ticket:       "kythe:#someParameter1",
			RelationKind: "/kythe/edge/param",
			Ordinal:      1,
		}},
	}
	expectedNodes := nodeInfos(getNodes(
		ticket,
		"kythe:#someRelatedNode",
		"kythe:#someParameter0",
		"kythe:#someParameter1"))

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		RelatedNodesByRelation: map[string]int64{
			"/kythe/edge/extends": 1,
			"/kythe/edge/param":   2,
		},
	}, reply.Total); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	} else if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	} else if err := testutil.DeepEqual(expectedNodes, reply.Nodes); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesMarkedSource(t *testing.T) {
	const ticket = "kythe://someCorpus?lang=otpl#withRelated"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket: []string{ticket},
		Filter: []string{"**"},
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,
		MarkedSource: &cpb.MarkedSource{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: "id",
		},

		RelatedNode: []*xpb.CrossReferencesReply_RelatedNode{{
			Ticket:       "kythe:#someRelatedNode",
			RelationKind: "/kythe/edge/extends",
		}, {
			Ticket:       "kythe:#someParameter0",
			RelationKind: "/kythe/edge/param",
			Ordinal:      0,
		}, {
			Ticket:       "kythe:#someParameter1",
			RelationKind: "/kythe/edge/param",
			Ordinal:      1,
		}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		RelatedNodesByRelation: map[string]int64{
			"/kythe/edge/extends": 1,
			"/kythe/edge/param":   2,
		},
	}, reply.Total); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	} else if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesMerge(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#withMerge"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:     []string{ticket},
		CallerKind: xpb.CrossReferencesRequest_DIRECT_CALLERS,
		Filter:     []string{"**"},
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,
		MarkedSource: &cpb.MarkedSource{
			Kind:    cpb.MarkedSource_IDENTIFIER,
			PreText: "id",
		},

		Caller: []*xpb.CrossReferencesReply_RelatedAnchor{{
			Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=someFile#someCallerAnchor",
				Parent: "kythe:?path=someFile",
				Span:   arbitrarySpan,
			},
			Ticket: "kythe:#someCaller",
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "id",
			},
			Site: []*xpb.Anchor{{
				Ticket: "kythe:?path=someFile#someCallsiteAnchor",
				Parent: "kythe:?path=someFile",
			}},
		}},
		RelatedNode: []*xpb.CrossReferencesReply_RelatedNode{{
			Ticket:       "kythe:#someRelatedNode",
			RelationKind: "/kythe/edge/extends",
		}, {
			Ticket:       "kythe:#someParameter0",
			RelationKind: "/kythe/edge/param",
			Ordinal:      0,
		}, {
			Ticket:       "kythe:#someParameter1",
			RelationKind: "/kythe/edge/param",
			Ordinal:      1,
		}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Callers: 1,
		RelatedNodesByRelation: map[string]int64{
			"/kythe/edge/extends": 1,
			"/kythe/edge/param":   2,
		},
	}, reply.Total); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	} else if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesIndirection(t *testing.T) {
	ticket := "kythe:#aliasNode"
	st := tbl.Construct(t)

	t.Run("none", func(t *testing.T) {
		experimentalCrossReferenceIndirectionKinds = nil

		reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
			Ticket:        []string{ticket},
			ReferenceKind: xpb.CrossReferencesRequest_ALL_REFERENCES,
		})
		testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

		expected := &xpb.CrossReferencesReply_CrossReferenceSet{
			Ticket: ticket,

			Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=somewhere#0-9",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=somewhere",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
				},
			}}},
		}

		if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
			References: 1,
			RefEdgeToCount: map[string]int64{
				"/kythe/edge/ref": 1,
			},
		}, reply.Total); err != nil {
			t.Error(err)
		}

		xr := reply.CrossReferences[ticket]
		if xr == nil {
			t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
		} else if err := testutil.DeepEqual(expected, xr); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("talias", func(t *testing.T) {
		// Enable indirection for talias nodes.
		experimentalCrossReferenceIndirectionKinds = nil
		experimentalCrossReferenceIndirectionKinds.Set("talias=%/kythe/edge/aliases")

		reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
			Ticket:        []string{ticket},
			ReferenceKind: xpb.CrossReferencesRequest_ALL_REFERENCES,
		})
		testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

		expected := &xpb.CrossReferencesReply_CrossReferenceSet{
			Ticket: ticket,

			Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=somewhere#0-9",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=somewhere",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=some/utf16/file#0-4",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=some/utf16/file",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe://c?path=/a/path",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   51,
						LineNumber:   4,
						ColumnOffset: 15,
					},
					End: &cpb.Point{
						ByteOffset:   55,
						LineNumber:   5,
						ColumnOffset: 2,
					},
				},
			}}},
		}

		if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
			References: 3,
			RefEdgeToCount: map[string]int64{
				"/kythe/edge/ref": 3,
			},
		}, reply.Total); err != nil {
			t.Error(err)
		}

		xr := reply.CrossReferences[ticket]
		if xr == nil {
			t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
		}

		sort.Sort(byOffset(xr.Reference))
		if err := testutil.DeepEqual(expected, xr); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("wildcard", func(t *testing.T) {
		// Enable indirection for any nodes with a reverse aliases edge.
		experimentalCrossReferenceIndirectionKinds = nil
		experimentalCrossReferenceIndirectionKinds.Set("*=%/kythe/edge/aliases")

		reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
			Ticket:        []string{ticket},
			ReferenceKind: xpb.CrossReferencesRequest_ALL_REFERENCES,
		})
		testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

		expected := &xpb.CrossReferencesReply_CrossReferenceSet{
			Ticket: ticket,

			Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=somewhere#0-9",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=somewhere",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=some/utf16/file#0-4",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=some/utf16/file",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe://c?path=/a/path",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   51,
						LineNumber:   4,
						ColumnOffset: 15,
					},
					End: &cpb.Point{
						ByteOffset:   55,
						LineNumber:   5,
						ColumnOffset: 2,
					},
				},
			}}},
		}

		if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
			References: 3,
			RefEdgeToCount: map[string]int64{
				"/kythe/edge/ref": 3,
			},
		}, reply.Total); err != nil {
			t.Error(err)
		}

		xr := reply.CrossReferences[ticket]
		if xr == nil {
			t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
		}

		sort.Sort(byOffset(xr.Reference))
		if err := testutil.DeepEqual(expected, xr); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("single_indirect", func(t *testing.T) {
		// Enable single indirection for talias nodes.
		experimentalCrossReferenceIndirectionKinds = nil
		experimentalCrossReferenceIndirectionKinds.Set("talias=/kythe/edge/indirect")

		reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
			Ticket:        []string{ticket},
			ReferenceKind: xpb.CrossReferencesRequest_ALL_REFERENCES,
		})
		testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

		expected := &xpb.CrossReferencesReply_CrossReferenceSet{
			Ticket: ticket,

			Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=somewhere#0-9",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=somewhere",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=somewhereElse#0-9",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=somewhereElse",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
				},
			}}},
		}

		if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
			References: 2,
			RefEdgeToCount: map[string]int64{
				"/kythe/edge/ref": 2,
			},
		}, reply.Total); err != nil {
			t.Error(err)
		}

		xr := reply.CrossReferences[ticket]
		if xr == nil {
			t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
		}

		sort.Sort(byOffset(xr.Reference))
		if err := testutil.DeepEqual(expected, xr); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("double_indirect", func(t *testing.T) {
		// Enable double indirection for talias nodes.
		experimentalCrossReferenceIndirectionKinds = nil
		experimentalCrossReferenceIndirectionKinds.Set("talias=/kythe/edge/indirect,indirect=/kythe/edge/indirect")

		reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
			Ticket:        []string{ticket},
			ReferenceKind: xpb.CrossReferencesRequest_ALL_REFERENCES,
		})
		testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

		expected := &xpb.CrossReferencesReply_CrossReferenceSet{
			Ticket: ticket,

			Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=somewhere#0-9",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=somewhere",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=somewhereElse#0-9",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=somewhereElse",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 9, LineNumber: 1, ColumnOffset: 9},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=some/utf16/file#0-4",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe:?path=some/utf16/file",

				Span: &cpb.Span{
					Start: &cpb.Point{LineNumber: 1},
					End:   &cpb.Point{ByteOffset: 4, LineNumber: 1, ColumnOffset: 4},
				},
			}}, {Anchor: &xpb.Anchor{
				Ticket: "kythe://c?lang=otpl?path=/a/path#51-55",
				Kind:   "/kythe/edge/ref",
				Parent: "kythe://c?path=/a/path",

				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   51,
						LineNumber:   4,
						ColumnOffset: 15,
					},
					End: &cpb.Point{
						ByteOffset:   55,
						LineNumber:   5,
						ColumnOffset: 2,
					},
				},
			}}},
		}

		if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
			References: 4,
			RefEdgeToCount: map[string]int64{
				"/kythe/edge/ref": 4,
			},
		}, reply.Total); err != nil {
			t.Error(err)
		}

		xr := reply.CrossReferences[ticket]
		if xr == nil {
			t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
		}

		sort.Sort(byOffset(xr.Reference))
		if err := testutil.DeepEqual(expected, xr); err != nil {
			t.Fatal(err)
		}
	})
}

func TestCrossReferencesDirectCallers(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#withCallers"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:     []string{ticket},
		CallerKind: xpb.CrossReferencesRequest_DIRECT_CALLERS,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Caller: []*xpb.CrossReferencesReply_RelatedAnchor{{
			Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=someFile#someCallerAnchor",
				Parent: "kythe:?path=someFile",
				Span:   arbitrarySpan,
			},
			Ticket: "kythe:#someCaller",
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "id",
			},
			Site: []*xpb.Anchor{{
				Ticket: "kythe:?path=someFile#someCallsiteAnchor",
				Parent: "kythe:?path=someFile",
			}},
		}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Callers: 1,
	}, reply.Total); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	} else if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesOverrideCallers(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#withCallers"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:     []string{ticket},
		CallerKind: xpb.CrossReferencesRequest_OVERRIDE_CALLERS,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply_CrossReferenceSet{
		Ticket: ticket,

		Caller: []*xpb.CrossReferencesReply_RelatedAnchor{{
			Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=someFile#someCallerAnchor",
				Parent: "kythe:?path=someFile",
				Span:   arbitrarySpan,
			},
			Ticket: "kythe:#someCaller",
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "id",
			},
			Site: []*xpb.Anchor{{
				Ticket: "kythe:?path=someFile#someCallsiteAnchor",
				Parent: "kythe:?path=someFile",
			}},
		}, {
			Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=someFile#someOverrideCallerAnchor1",
				Parent: "kythe:?path=someFile",
				Span:   arbitrarySpan,
			},
			Site: []*xpb.Anchor{{
				Ticket: "kythe:?path=someFile#someCallsiteAnchor",
				Parent: "kythe:?path=someFile",
				Span:   arbitrarySpan,
			}},
		}, {
			Anchor: &xpb.Anchor{
				Ticket: "kythe:?path=someFile#someOverrideCallerAnchor2",
				Parent: "kythe:?path=someFile",
				Span:   arbitrarySpan,
			},
			Site: []*xpb.Anchor{{
				Ticket: "kythe:?path=someFile#someCallsiteAnchor",
				Parent: "kythe:?path=someFile",
			}},
		}},
	}

	if err := testutil.DeepEqual(&xpb.CrossReferencesReply_Total{
		Callers: 3,
	}, reply.Total); err != nil {
		t.Error(err)
	}

	xr := reply.CrossReferences[ticket]
	if xr == nil {
		t.Fatalf("Missing expected CrossReferences; found: %#v", reply)
	} else if err := testutil.DeepEqual(expected, xr); err != nil {
		t.Fatal(err)
	}
}

func TestCrossReferencesRevisions(t *testing.T) {
	ticket := "kythe://someCorpus?lang=otpl#withInfos"

	st := tbl.Construct(t)
	reply, err := st.CrossReferences(ctx, &xpb.CrossReferencesRequest{
		Ticket:         []string{ticket},
		DefinitionKind: xpb.CrossReferencesRequest_ALL_DEFINITIONS,
		ReferenceKind:  xpb.CrossReferencesRequest_ALL_REFERENCES,
		Snippets:       xpb.SnippetsKind_NONE,
	})
	testutil.Fatalf(t, "CrossReferencesRequest error: %v", err)

	expected := &xpb.CrossReferencesReply{
		Total: &xpb.CrossReferencesReply_Total{
			References: 1,
			RefEdgeToCount: map[string]int64{
				"/kythe/edge/ref": 1,
			},
		},
		Filtered: &xpb.CrossReferencesReply_Total{},
		CrossReferences: map[string]*xpb.CrossReferencesReply_CrossReferenceSet{
			ticket: &xpb.CrossReferencesReply_CrossReferenceSet{
				Ticket: ticket,

				Reference: []*xpb.CrossReferencesReply_RelatedAnchor{{
					Anchor: &xpb.Anchor{
						Ticket:   "kythe://corpus?lang=otpl?path=some/file#27-33",
						Kind:     "/kythe/edge/ref",
						Parent:   "kythe://corpus?path=some/file",
						Revision: "someFileRev", // ⟵ this is expected now

						Span: &cpb.Span{
							Start: &cpb.Point{
								ByteOffset:   27,
								LineNumber:   2,
								ColumnOffset: 10,
							},
							End: &cpb.Point{
								ByteOffset:   33,
								LineNumber:   3,
								ColumnOffset: 5,
							},
						},
					},
				}},
			},
		},
	}

	if diff := compare.ProtoDiff(expected, reply); diff != "" {
		t.Fatalf("(-expected; +found):\n%s", diff)
	}
}

func nodeInfos(nss ...[]*srvpb.Node) map[string]*cpb.NodeInfo {
	m := make(map[string]*cpb.NodeInfo)
	for _, ns := range nss {
		for _, n := range ns {
			if ni := nodeInfo(n); ni != nil {
				m[n.Ticket] = ni
			}
		}
	}
	return m
}

func TestDocumentationEmpty(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Documentation(ctx, &xpb.DocumentationRequest{
		Ticket: []string{"kythe:#undocumented"},
	})

	expected := &xpb.DocumentationReply{}

	if reply == nil || err != nil {
		t.Fatalf("Documentation call failed: (reply: %v; error: %v)", reply, err)
	} else if err := testutil.DeepEqual(expected, reply); err != nil {
		t.Fatal(err)
	}
}

func TestDocumentation(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Documentation(ctx, &xpb.DocumentationRequest{
		Ticket: []string{"kythe:#documented"},
	})

	expected := &xpb.DocumentationReply{
		Document: []*xpb.DocumentationReply_Document{{
			Ticket: "kythe:#documented",
			Text: &xpb.Printable{
				RawText: "some documentation text",
			},
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "DocumentBuilderFactory",
			},
		}},
		Nodes: nodeInfos(getNodes("kythe:#documented")),
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:?path=def/location#defDoc": &xpb.Anchor{
				Ticket: "kythe:?path=def/location#defDoc",
				Parent: "kythe:?path=def/location",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   1,
						LineNumber:   1,
						ColumnOffset: 1,
					},
					End: &cpb.Point{
						ByteOffset:   4,
						LineNumber:   1,
						ColumnOffset: 4,
					},
				},
			},
		},
	}

	if reply == nil || err != nil {
		t.Fatalf("Documentation call failed: (reply: %v; error: %v)", reply, err)
	} else if diff := compare.ProtoDiff(expected, reply); diff != "" {
		t.Fatalf("(-expected; +found):\n%s", diff)
	}
}

func TestDocumentationChildren(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Documentation(ctx, &xpb.DocumentationRequest{
		Ticket: []string{"kythe:#documented"},

		IncludeChildren: true,
	})

	expected := &xpb.DocumentationReply{
		Document: []*xpb.DocumentationReply_Document{{
			Ticket: "kythe:#documented",
			Text: &xpb.Printable{
				RawText: "some documentation text",
			},
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "DocumentBuilderFactory",
			},
			Children: []*xpb.DocumentationReply_Document{{
				Ticket: "kythe:#childDoc",
				Text: &xpb.Printable{
					RawText: "child document text",
				},
			}, {
				Ticket: "kythe:#childDocBy",
				Text: &xpb.Printable{
					RawText: "second child document text",
				},
			}},
		}},
		Nodes: nodeInfos(getNodes(
			"kythe:#childDoc",
			"kythe:#childDocBy",
			"kythe:#documented",
			"kythe:#secondChildDoc",
		)),
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:?path=def/location#defDoc": &xpb.Anchor{
				Ticket: "kythe:?path=def/location#defDoc",
				Parent: "kythe:?path=def/location",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   1,
						LineNumber:   1,
						ColumnOffset: 1,
					},
					End: &cpb.Point{
						ByteOffset:   4,
						LineNumber:   1,
						ColumnOffset: 4,
					},
				},
			},
		},
	}

	if reply == nil || err != nil {
		t.Fatalf("Documentation call failed: (reply: %v; error: %v)", reply, err)
	} else if diff := compare.ProtoDiff(expected, reply); diff != "" {
		t.Fatalf("(-expected; +found):\n%s", diff)
	}
}

func TestDocumentationPatching(t *testing.T) {
	st := tbl.Construct(t)

	patcher := &mockPatcher{}
	st.MakePatcher = func(ctx context.Context, ws *xpb.Workspace) (MultiFilePatcher, error) {
		return patcher, nil
	}

	reply, err := st.Documentation(ctx, &xpb.DocumentationRequest{
		Ticket: []string{"kythe:#documented"},

		IncludeChildren: true,

		Workspace:             &xpb.Workspace{Uri: "test:"},
		PatchAgainstWorkspace: true,
	})

	expected := &xpb.DocumentationReply{
		Document: []*xpb.DocumentationReply_Document{{
			Ticket: "kythe:#documented",
			Text: &xpb.Printable{
				RawText: "some documentation text",
			},
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "DocumentBuilderFactory",
			},
			Children: []*xpb.DocumentationReply_Document{{
				Ticket: "kythe:#childDoc",
				Text: &xpb.Printable{
					RawText: "child document text",
				},
			}, {
				Ticket: "kythe:#childDocBy",
				Text: &xpb.Printable{
					RawText: "second child document text",
				},
			}},
		}},
		Nodes: nodeInfos(getNodes(
			"kythe:#childDoc",
			"kythe:#childDocBy",
			"kythe:#documented",
			"kythe:#secondChildDoc",
		)),
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:?path=def/location#defDoc": &xpb.Anchor{
				Ticket: "kythe:?path=def/location#defDoc",
				Parent: "kythe:?path=def/location",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   2,
						LineNumber:   2,
						ColumnOffset: 2,
					},
					End: &cpb.Point{
						ByteOffset:   5,
						LineNumber:   2,
						ColumnOffset: 5,
					},
				},
			},
		},
	}

	if reply == nil || err != nil {
		t.Fatalf("Documentation call failed: (reply: %v; error: %v)", reply, err)
	} else if diff := compare.ProtoDiff(expected, reply); diff != "" {
		t.Fatalf("(-expected; +found):\n%s", diff)
	}
}

func TestDocumentationIndirection(t *testing.T) {
	st := tbl.Construct(t)
	reply, err := st.Documentation(ctx, &xpb.DocumentationRequest{
		Ticket: []string{"kythe:#documentedBy"},
	})

	expected := &xpb.DocumentationReply{
		Document: []*xpb.DocumentationReply_Document{{
			Ticket: "kythe:#documentedBy",
			Text: &xpb.Printable{
				RawText: "some documentation text",
			},
			MarkedSource: &cpb.MarkedSource{
				Kind:    cpb.MarkedSource_IDENTIFIER,
				PreText: "DocumentBuilderFactory",
			},
		}},
		Nodes: nodeInfos(getNodes("kythe:#documented", "kythe:#documentedBy")),
		DefinitionLocations: map[string]*xpb.Anchor{
			"kythe:?path=def/location#defDoc": &xpb.Anchor{
				Ticket: "kythe:?path=def/location#defDoc",
				Parent: "kythe:?path=def/location",
				Span: &cpb.Span{
					Start: &cpb.Point{
						ByteOffset:   1,
						LineNumber:   1,
						ColumnOffset: 1,
					},
					End: &cpb.Point{
						ByteOffset:   4,
						LineNumber:   1,
						ColumnOffset: 4,
					},
				},
			},
		},
	}

	if reply == nil || err != nil {
		t.Fatalf("Documentation call failed: (reply: %v; error: %v)", reply, err)
	} else if diff := compare.ProtoDiff(expected, reply); diff != "" {
		t.Fatalf("(-expected; +found):\n%s", diff)
	}
}

func TestPageSearchIndex(t *testing.T) {
	set := &srvpb.PagedCrossReferences{
		PageSearchIndex: &srvpb.PagedCrossReferences_PageSearchIndex{
			ByCorpus: &srvpb.PagedCrossReferences_PageSearchIndex_Postings{
				Index: map[uint32]*srvpb.PagedCrossReferences_PageSearchIndex_Pages{
					tri("kyt"): pages(0),
					tri("the"): pages(0, 1, 2),
					tri("yth"): pages(0),
					tri("oth"): pages(1),
					tri("her"): pages(1),
					tri("Pag"): pages(math.MaxUint32), // short-hand for all pages
					tri("age"): pages(0, 1, 2),
					tri("ge_"): pages(1, 2),
				},
			},
			ByRoot: &srvpb.PagedCrossReferences_PageSearchIndex_Postings{
				Index: map[uint32]*srvpb.PagedCrossReferences_PageSearchIndex_Pages{
					tri("baz"): pages(1, 2),
					tri("aze"): pages(1, 2),
					tri("zel"): pages(1, 2),
				},
			},
			ByPath: &srvpb.PagedCrossReferences_PageSearchIndex_Postings{
				Index: map[uint32]*srvpb.PagedCrossReferences_PageSearchIndex_Pages{
					tri("kyt"): pages(0),
					tri("yth"): pages(0),
					tri("the"): pages(0),
					tri("he/"): pages(0),
					tri("e/g"): pages(0),
					tri("/go"): pages(0),
				},
			},
			ByResolvedPath: &srvpb.PagedCrossReferences_PageSearchIndex_Postings{
				Index: map[uint32]*srvpb.PagedCrossReferences_PageSearchIndex_Pages{
					tri("the"): pages(0, 2),
					tri("he/"): pages(0, 2),
					tri("e/b"): pages(0, 2),
					tri("/ba"): pages(0, 2),
				},
			},
		},
		PageIndex: []*srvpb.PagedCrossReferences_PageIndex{{
			PageKey: "kythePage",
		}, {
			PageKey: "otherPage_",
		}, {
			PageKey: "thePage_",
		}},
	}

	// All pages are represented by nil
	var allPages stringset.Set

	tests := []struct {
		Filter *xpb.CorpusPathFilters
		Keys   stringset.Set
	}{
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kyt" }`), stringset.New("kythePage")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kythe" }`), stringset.New("kythePage")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kythe|golang" }`), stringset.New("kythePage")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "other" }`), stringset.New("otherPage_")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kythe|other" }`), stringset.New("kythePage", "otherPage_")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "Page_" }`), stringset.New("thePage_", "otherPage_")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "the" }`), allPages},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "Page" }`), allPages},

		// Trigrams are separated by field
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kythe/go" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "bazel" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "the/ba" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY root: "the/ba" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY path: "the/ba" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY path: "kythe/go" }`), stringset.New("kythePage")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY path: "/go" }`), stringset.New("kythePage")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY root: "bazel" }`), stringset.New("otherPage_", "thePage_")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY resolved_path: "the/ba" }`), stringset.New("kythePage", "thePage_")},

		// Merge filters
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kythe|other" } filter: { type: INCLUDE_ONLY corpus: "other" }`), stringset.New("otherPage_")},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY resolved_path: "the/ba" } filter: { type: INCLUDE_ONLY root: "bazel" }`), stringset.New("thePage_")},

		// No trigrams; matches everything
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "ne" }`), allPages},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "^$" }`), allPages},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "\\.s" }`), allPages},

		// Matches nothing
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "nope" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "rip" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kyther" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kythe.+google" }`), stringset.New()},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "golang" }`), stringset.New()},

		// Exclusion filter are not handled by the search index; they match all pages.
		{mustParseFilters(`filter: { type: EXCLUDE corpus: "kythe" }`), allPages},
	}

	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			t.Logf("CorpusPathFilters: %s", test.Filter)
			filter, err := compileCorpusPathFilters(test.Filter, nil)
			testutil.Fatalf(t, "compileCorpusPathFilters: %v", err)
			found := filter.PageSet(set)

			var expected *pageSet
			if test.Keys != nil {
				expected = &pageSet{KeySet: test.Keys}
			}
			if diff := compare.ProtoDiff(expected, found); diff != "" {
				t.Fatalf("Unexpected diff (-expected; +found):\n%s", diff)
			}
		})
	}
}

func pages(ps ...uint32) *srvpb.PagedCrossReferences_PageSearchIndex_Pages {
	if len(ps) <= 1 {
		return &srvpb.PagedCrossReferences_PageSearchIndex_Pages{PageIndex: ps}
	}
	encoded := make([]uint32, len(ps))
	encoded[0] = ps[0]
	for i, n := range ps[1:] {
		encoded[i+1] = n - encoded[i]
	}
	return &srvpb.PagedCrossReferences_PageSearchIndex_Pages{PageIndex: encoded}
}

func TestCorpusPathFilters(t *testing.T) {
	tests := []struct {
		filters *xpb.CorpusPathFilters

		includes, excludes []string
	}{
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "^kythe3" }`),
			[]string{cps("kythe3", "", ""), cps("kythe3//branch", "", "")},
			[]string{cps("other", "", ""), cps("other", "kythe3", "kythe3")}},
		{mustParseFilters(`filter: { type: EXCLUDE root: ".+" }`),
			[]string{cps("kythe3", "", ""), cps("kythe3//branch", "", ""), cps("oss", "", "any/path")},
			[]string{cps("kythe3", "genfiles", ""), cps("other", "bin", "some/path")}},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "^kythe3" root: "genfiles" }`),
			[]string{cps("kythe3", "genfiles", ""), cps("kythe3//branch", "genfiles", "")},
			[]string{cps("kythe2", "genfiles", ""), cps("kythe3", "bin", "path"), cps("other", "bin", "some/path")}},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "kythe" } filter: { type: INCLUDE_ONLY corpus: "2|3" }`),
			[]string{cps("kythe3", "genfiles", "path"), cps("kythe2", "", "blah")},
			[]string{cps("kythe4", "", ""), cps("kythe", "kythe2", "kythe3")}},
		{mustParseFilters(`filter: { type: EXCLUDE root: ".+" } filter: { type: INCLUDE_ONLY corpus: "^kythe3"}`),
			[]string{cps("kythe3", "", "any/path"), cps("kythe3//branch", "", "some/path")},
			[]string{cps("kythe3", "genfiles", "any/path"), cps("kythe3//branch", "bin", "some/path")}},
		{mustParseFilters(`filter: { type: INCLUDE_ONLY resolved_path: "^kythe3/branch/genfiles/"}`),
			[]string{cps("kythe3//branch", "genfiles", "any/path"), cps("kythe3//branch", "genfiles/more", "any/path")},
			[]string{cps("kythe3", "bin", "any/path"), cps("kythe3", "genfiles", "some/path")}},
		// The filter should only apply when the corpus matches.
		{mustParseFilters(`filter: { type: INCLUDE_ONLY corpus: "^kythe3" path: ".*k.*" corpus_specific_filter: true}`),
			[]string{cps("kythe3", "", "k1.cc"), cps("kythe3//branch", "", "k3.cc"), cps("other", "", "file.cc")},
			[]string{cps("kythe3", "", "file.cc")}},
	}

	for i, test := range tests {
		t.Run(strconv.Itoa(i), func(t *testing.T) {
			f, err := compileCorpusPathFilters(test.filters, nil)
			testutil.Fatalf(t, "Error: %v", err)

			for _, include := range test.includes {
				if !f.AllowTicket(include) {
					t.Errorf("Expected %q to be included but it wasn't", include)
				}
			}
			for _, exclude := range test.excludes {
				if f.AllowTicket(exclude) {
					t.Errorf("Expected %q to be excluded but it wasn't", exclude)
				}
			}
		})
	}
}

func randPostings(rand *rand.Rand) *srvpb.PagedCrossReferences_PageSearchIndex_Postings {
	numPages := rand.Intn(24) + 1
	p := &srvpb.PagedCrossReferences_PageSearchIndex_Postings{
		Index: make(map[uint32]*srvpb.PagedCrossReferences_PageSearchIndex_Pages, numPages),
	}
	n := rand.Intn(32) + 1
	for j := 0; j < n; j++ {
		s := randTrigram(rand)
		t := tri(s)
		ps := make([]uint32, rand.Intn(numPages))
		if len(ps) == 0 {
			ps = append([]uint32{}, allPages...)
		} else {
			for k := 0; k < len(ps); k++ {
				ps[k] = uint32(rand.Intn(numPages))
			}

			// Sort and dedup the pages
			sort.Slice(ps, func(i, j int) bool { return ps[i] < ps[j] })
			var k int
			for l := 1; l < len(ps); l++ {
				if ps[l-1] == ps[l] {
					continue
				}
				ps[k] = ps[l]
				k++
			}
			ps = ps[:k]
		}
		p.Index[t] = pages(ps...)
	}
	return p
}

func randQuery(rand *rand.Rand) []*index.Query {
	qs := make([]*index.Query, rand.Intn(4)+1)
	if rand.Intn(1000) == 0 {
		return nil
	}
	for j := 0; j < len(qs); j++ {
		q := &index.Query{}
		o := rand.Intn(100)
		switch {
		case o == 0:
			q.Op = index.QAll
		case o == 99:
			q.Op = index.QNone
		case o < 50:
			q.Op = index.QAnd
		default:
			q.Op = index.QOr
		}
		if q.Op == index.QAll || q.Op == index.QOr {
			n := rand.Intn(4) + 1
			for k := 0; k < n; k++ {
				q.Trigram = append(q.Trigram, randTrigram(rand))
			}
		}
		if rand.Intn(10) == 0 {
			q.Sub = randQuery(rand)
		}
		qs[j] = q
	}
	return qs
}

func TestApplyQueries(t *testing.T) {
	testutil.Fatalf(t, "Error: %v", quick.Check(func(p *srvpb.PagedCrossReferences_PageSearchIndex_Postings, qs []*index.Query) bool {
		res := applyQueries(p, qs, nil)
		if len(res) > 1 {
			for _, x := range res {
				// Make sure the result doesn't include the allPages marker.
				if x == math.MaxUint32 {
					t.Logf("Postings: %s", p)
					t.Logf("Query: %s", qs)
					t.Logf("Result: %v", res)
					return false
				}
			}
		}
		return true
	}, &quick.Config{Values: func(args []reflect.Value, rand *rand.Rand) {
		args[0] = reflect.ValueOf(randPostings(rand))
		args[1] = reflect.ValueOf(randQuery(rand))
	}}))
}

func randTrigram(rand *rand.Rand) string {
	return string([]rune{rune(rand.Intn(8)) + 'a', rune(rand.Intn(8)) + 'a', rune(rand.Intn(8)) + 'a'})
}

func cps(corpus, root, path string) string {
	u := &kytheuri.URI{Corpus: corpus, Root: root, Path: path}
	return u.String()
}

func mustParseFilters(msg string) *xpb.CorpusPathFilters {
	var f xpb.CorpusPathFilters
	if err := prototext.Unmarshal([]byte(msg), &f); err != nil {
		panic(err)
	}
	return &f
}

// byOffset implements the sort.Interface for *xpb.CrossReferencesReply_RelatedAnchors.
type byOffset []*xpb.CrossReferencesReply_RelatedAnchor

// Implement the sort.Interface.
func (s byOffset) Len() int      { return len(s) }
func (s byOffset) Swap(i, j int) { s[i], s[j] = s[j], s[i] }
func (s byOffset) Less(i, j int) bool {
	if s[i].Anchor.Span.Start.ByteOffset != s[j].Anchor.Span.Start.ByteOffset {
		return s[i].Anchor.Span.Start.ByteOffset < s[j].Anchor.Span.Start.ByteOffset
	} else if len(s[i].Site) == 0 || len(s[i].Site) != len(s[j].Site) {
		return len(s[i].Site) < len(s[j].Site)
	}
	return s[i].Site[0].Span.Start.ByteOffset < s[j].Site[0].Span.Start.ByteOffset
}

func nodeInfo(n *srvpb.Node) *cpb.NodeInfo {
	ni := &cpb.NodeInfo{
		Facts:      make(map[string][]byte, len(n.Fact)),
		Definition: n.DefinitionLocation.GetTicket(),
	}
	for _, f := range n.Fact {
		ni.Facts[f.Name] = f.Value
	}
	if len(ni.Facts) == 0 && ni.Definition == "" {
		return nil
	}
	return ni
}

func makeFactList(keyVals ...string) []*cpb.Fact {
	if len(keyVals)%2 != 0 {
		panic("makeFactList: odd number of key values")
	}
	facts := make([]*cpb.Fact, 0, len(keyVals)/2)
	for i := 0; i < len(keyVals); i += 2 {
		facts = append(facts, &cpb.Fact{
			Name:  keyVals[i],
			Value: []byte(keyVals[i+1]),
		})
	}
	return facts
}

func refs(norm *span.Normalizer, ds []*srvpb.FileDecorations_Decoration, infos []*srvpb.FileInfo) (refs []*xpb.DecorationsReply_Reference) {
	fileInfos := makeFileInfoMap(infos)
	for _, d := range ds {
		r := decorationToReference(norm, d)
		r.TargetRevision = fileInfos[r.TargetTicket].GetRevision()
		refs = append(refs, r)
	}
	return
}

type testTable struct {
	Nodes       []*srvpb.Node
	Decorations []*srvpb.FileDecorations
	RefSets     []*srvpb.PagedCrossReferences
	RefPages    []*srvpb.PagedCrossReferences_Page
	Documents   []*srvpb.Document
}

func (tbl *testTable) Construct(t *testing.T) *Table {
	p := make(testProtoTable)
	for _, d := range tbl.Decorations {
		testutil.Fatalf(t, "Error writing file decorations: %v", p.Put(ctx, DecorationsKey(mustFix(t, d.File.Ticket)), d))
	}
	for _, cr := range tbl.RefSets {
		testutil.Fatalf(t, "Error writing cross-references: %v", p.Put(ctx, CrossReferencesKey(mustFix(t, cr.SourceTicket)), cr))
	}
	for _, crp := range tbl.RefPages {
		testutil.Fatalf(t, "Error writing cross-references: %v", p.Put(ctx, CrossReferencesPageKey(crp.PageKey), crp))
	}
	for _, doc := range tbl.Documents {
		testutil.Fatalf(t, "Error writing documents: %v", p.Put(ctx, DocumentationKey(doc.Ticket), doc))
	}
	return NewCombinedTable(p)
}

func mustFix(t *testing.T, ticket string) string {
	ft, err := kytheuri.Fix(ticket)
	if err != nil {
		t.Fatalf("Error fixing ticket %q: %v", ticket, err)
	}
	return ft
}

type testProtoTable map[string]proto.Message

func (t testProtoTable) Put(_ context.Context, key []byte, val proto.Message) error {
	t[string(key)] = val
	return nil
}

func (t testProtoTable) Lookup(_ context.Context, key []byte, msg proto.Message) error {
	m, ok := t[string(key)]
	if !ok {
		return table.ErrNoSuchKey
	}
	proto.Merge(msg, m)
	return nil
}

func (t testProtoTable) LookupValues(_ context.Context, key []byte, m proto.Message, f func(proto.Message) error) error {
	val, ok := t[string(key)]
	if !ok {
		return nil
	}
	msg := m.ProtoReflect().New().Interface()
	proto.Merge(msg, val)
	if err := f(msg); err != nil && err != table.ErrStopLookup {
		return err
	}
	return nil
}

func (t testProtoTable) Buffered() table.BufferedProto { panic("UNIMPLEMENTED") }

func (t testProtoTable) Close(_ context.Context) error { return nil }

func fi(cp *cpb.CorpusPath, rev string) *srvpb.FileInfo {
	return &srvpb.FileInfo{
		CorpusPath: cp,
		Revision:   rev,
	}
}

func cp(corpus, root, path string) *cpb.CorpusPath {
	return &cpb.CorpusPath{
		Corpus: corpus,
		Root:   root,
		Path:   path,
	}
}
