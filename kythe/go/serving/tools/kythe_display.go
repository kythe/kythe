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

package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"kythe/go/services/xrefs"
	"kythe/go/util/kytheuri"
	"kythe/go/util/schema"

	srvpb "kythe/proto/serving_proto"
	spb "kythe/proto/storage_proto"
	xpb "kythe/proto/xref_proto"
)

var (
	displayJSON = flag.Bool("json", false, "Display results as JSON")
	out         = os.Stdout
)

func displayCorpusRoots(cr *srvpb.CorpusRoots) error {
	if *displayJSON {
		return json.NewEncoder(out).Encode(cr)
	}

	for _, c := range cr.Corpus {
		for _, root := range c.Root {
			var err error
			if lsURIs {
				uri := kytheuri.URI{
					Corpus: c.Corpus,
					Root:   root,
				}
				_, err = fmt.Fprintln(out, uri.String())
			} else {
				_, err = fmt.Fprintln(out, filepath.Join(c.Corpus, root))
			}
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func displayDirectory(d *srvpb.FileDirectory) error {
	if *displayJSON {
		return json.NewEncoder(out).Encode(d)
	}

	for _, d := range d.Subdirectory {
		if !lsURIs {
			uri, err := kytheuri.Parse(d)
			if err != nil {
				return fmt.Errorf("received invalid directory uri %q: %v", d, err)
			}
			d = filepath.Base(uri.Path) + "/"
		}
		if _, err := fmt.Fprintln(out, d); err != nil {
			return err
		}
	}
	for _, f := range d.FileTicket {
		if !lsURIs {
			uri, err := kytheuri.Parse(f)
			if err != nil {
				return fmt.Errorf("received invalid file ticket %q: %v", f, err)
			}
			f = filepath.Base(uri.Path)
		}
		if _, err := fmt.Fprintln(out, f); err != nil {
			return err
		}
	}
	return nil
}

func displaySource(decor *xpb.DecorationsReply) error {
	if *displayJSON {
		return json.NewEncoder(out).Encode(decor)
	}

	_, err := out.Write(decor.SourceText)
	return err
}

func displayReferences(decor *xpb.DecorationsReply) error {
	if *displayJSON {
		return json.NewEncoder(out).Encode(decor)
	}

	nodes := xrefs.NodesMap(decor.Node)

	for _, ref := range decor.Reference {
		nodeKind := factValue(nodes, ref.TargetTicket, schema.NodeKindFact, "UNKNOWN")
		subkind := factValue(nodes, ref.TargetTicket, schema.SubkindFact, "")
		startOffset := factValue(nodes, ref.SourceTicket, schema.AnchorStartFact, "_")
		endOffset := factValue(nodes, ref.SourceTicket, schema.AnchorEndFact, "_")
		r := strings.NewReplacer(
			"@source@", ref.SourceTicket,
			"@target@", ref.TargetTicket,
			"@edgeKind@", ref.Kind,
			"@nodeKind@", nodeKind,
			"@subkind@", subkind,
			"@beg@", startOffset,
			"@end@", endOffset,
		)
		if _, err := r.WriteString(out, refFormat+"\n"); err != nil {
			return err
		}
	}

	return nil
}

func displayEdges(edges *xpb.EdgesReply) error {
	if *displayJSON {
		return json.NewEncoder(out).Encode(edges)
	}

	for _, es := range edges.EdgeSet {
		if _, err := fmt.Fprintln(out, "source:", es.SourceTicket); err != nil {
			return err
		}
		for _, g := range es.Group {
			for _, target := range g.TargetTicket {
				if _, err := fmt.Fprintf(out, "%s\t%s\n", g.Kind, target); err != nil {
					return err
				}
			}
		}
	}
	return nil
}

func displayEdgeCounts(edges *xpb.EdgesReply) error {
	counts := make(map[string]int)
	for _, es := range edges.EdgeSet {
		for _, g := range es.Group {
			counts[g.Kind] += len(g.TargetTicket)
		}
	}

	if *displayJSON {
		return json.NewEncoder(out).Encode(counts)
	}

	for kind, cnt := range counts {
		if _, err := fmt.Fprintf(out, "%s\t%d\n", kind, cnt); err != nil {
			return err
		}
	}
	return nil
}

func displayNodes(nodes []*xpb.NodeInfo) error {
	if *displayJSON {
		return json.NewEncoder(out).Encode(nodes)
	}

	for _, n := range nodes {
		if _, err := fmt.Fprintln(out, n.Ticket); err != nil {
			return err
		}
		for _, fact := range n.Fact {
			if len(fact.Value) <= factSizeThreshold {
				if _, err := fmt.Fprintf(out, "  %s\t%s\n", fact.Name, fact.Value); err != nil {
					return err
				}
			} else {
				if _, err := fmt.Fprintf(out, "  %s\n", fact.Name); err != nil {
					return err
				}
			}
		}
	}
	return nil
}

func displaySearch(reply *spb.SearchReply) error {
	if *displayJSON {
		return json.NewEncoder(out).Encode(reply)
	}

	for _, t := range reply.Ticket {
		if _, err := fmt.Fprintln(out, t); err != nil {
			return err
		}
	}
	fmt.Fprintln(os.Stderr, "Total Results:", len(reply.Ticket))
	return nil
}

func factValue(m map[string]map[string][]byte, ticket, factName, def string) string {
	if n, ok := m[ticket]; ok {
		if val, ok := n[factName]; ok {
			return string(val)
		}
	}
	return def
}
