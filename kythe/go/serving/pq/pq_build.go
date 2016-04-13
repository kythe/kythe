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

package pq

import (
	"database/sql"
	"fmt"
	"log"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/services/xrefs"
	"kythe.io/kythe/go/serving/xrefs/assemble"
	"kythe.io/kythe/go/util/kytheuri"
	"kythe.io/kythe/go/util/schema"

	"github.com/golang/protobuf/proto"
	"github.com/lib/pq"

	cpb "kythe.io/kythe/proto/common_proto"
	srvpb "kythe.io/kythe/proto/serving_proto"
	spb "kythe.io/kythe/proto/storage_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

const (
	dropTables = `DROP TABLE IF EXISTS Nodes, Edges, CrossReferences, Files CASCADE;`

	// TODO(schroederc): look into using an HSTORE or JSONB column
	// TODO(schroederc): split anchor / text facts into separate tables

	// Table of node facts keyed by their Kythe URI (ticket).  Common facts have
	// their own columns; other facts are left in the other_facts column (as a
	// wire-encoded srvpb.Node).
	createNodesTable = `
CREATE TABLE Nodes (
ticket text NOT NULL,
node_kind text NOT NULL,
subkind text,
text bytea,
text_encoding text,
start_offset int,
end_offset int,
snippet_start int,
snippet_end int,
other_facts_num int NOT NULL,
other_facts bytea,
PRIMARY KEY (ticket));`

	// Table of unique Kythe edges.
	createEdgeTable = `
CREATE TABLE Edges (
source text NOT NULL,
kind text NOT NULL,
target text NOT NULL,
ordinal int NOT NULL,
PRIMARY KEY (source, kind, target, ordinal));`

	// TODO(schroederc): possibly materialize the AllEdges table (or insert its data into Edges)
	createNodeEdgeIndices = `
CREATE INDEX node_kinds ON nodes (node_kind);

CREATE INDEX edge_targets ON edges (target);
CREATE VIEW AllEdges AS SELECT * FROM Edges UNION ALL (SELECT target, CONCAT('%', kind), source, ordinal FROM Edges);`

	// Table of xpb.DecorationReply_References.
	createDecorationsTable = `
CREATE MATERIALIZED VIEW
Decorations (file_ticket, anchor_ticket, start_offset, end_offset, kind, target_ticket) AS
SELECT fileNode.ticket file_ticket, anchorNode.ticket anchor_ticket, anchorNode.start_offset, anchorNode.end_offset, edge.kind, edge.target
FROM Edges edge
JOIN Nodes anchorNode  ON edge.source        = anchorNode.ticket
JOIN Edges childofEdge ON anchorNode.ticket  = childofEdge.source
JOIN Nodes fileNode    ON childofEdge.target = fileNode.ticket
WHERE fileNode.node_kind = 'file'
AND anchorNode.node_kind = 'anchor'
AND childofEdge.kind = '/kythe/edge/childof'
AND edge.kind <> '/kythe/edge/childof';`

	// Table of wire-encoded xpb.Anchor protos for CrossReferences.
	createCrossReferencesTable = `
CREATE TABLE CrossReferences (
ticket text NOT NULL,
kind text NOT NULL,
file_ticket text NOT NULL,
anchor_ticket text NOT NULL,
proto bytea NOT NULL);`

	// Table of file tree data for CorpusRoots/Directory calls.
	createFilesTable = `
CREATE TABLE Files (
corpus text NOT NULL,
root text NOT NULL,
path text NOT NULL,
ticket text NOT NULL,
file bool NOT NULL,
PRIMARY KEY (corpus, root, path, ticket));`

	createXRefIndices = `
CREATE UNIQUE INDEX decorations_uniq_index ON decorations (file_ticket, anchor_ticket, kind, target_ticket);
CREATE INDEX decorations_lookup_index ON decorations (file_ticket);

CREATE INDEX cross_references_lookup_index ON crossreferences (ticket, kind);`
)

// CopyEntries completely recreates the database using the given stream of
// GraphStore-ordered entries.
func (d *DB) CopyEntries(entries <-chan *spb.Entry) error {
	allStart := time.Now()

	start := time.Now()
	if _, err := d.Exec(dropTables); err != nil {
		return fmt.Errorf("error dropping existing tables: %v", err)
	}
	log.Printf("Dropped existing tables in %v", time.Since(start))

	start = time.Now()
	if err := d.copyEntries(entries); err != nil {
		return err
	}
	log.Printf("Completed entries copy in %v", time.Since(start))

	start = time.Now()
	if _, err := d.Exec(createNodeEdgeIndices); err != nil {
		return fmt.Errorf("error creating node/edge indices: %v", err)
	}
	log.Printf("Created Node/Edge indices in %v", time.Since(start))

	start = time.Now()
	if err := d.createFileTree(); err != nil {
		return fmt.Errorf("error creating file tree: %v", err)
	}
	log.Printf("Constructed Files table in %v", time.Since(start))

	start = time.Now()
	if _, err := d.Exec(createDecorationsTable); err != nil {
		return fmt.Errorf("error creating Decorations table: %v", err)
	}
	log.Printf("Constructed Decorations table in %v", time.Since(start))

	start = time.Now()
	if err := d.createCrossRefsTable(); err != nil {
		return fmt.Errorf("error creating CrossReferences table: %v", err)
	}
	log.Printf("Constructed CrossReferences table in %v", time.Since(start))

	start = time.Now()
	if _, err := d.Exec(createXRefIndices); err != nil {
		return fmt.Errorf("error creating xref indices: %v", err)
	}
	log.Printf("Created XRef indices in %v", time.Since(start))

	start = time.Now()
	if _, err := d.Exec("VACUUM ANALYZE"); err != nil {
		return fmt.Errorf("error vacuuming db: %v", err)
	}
	log.Printf("Analyzed database in %v", time.Since(start))

	log.Printf("Recreated all tables in %v", time.Since(allStart))
	return nil
}

func (d *DB) copyEntries(entries <-chan *spb.Entry) error {
	// Start a transaction for a COPY statement per table
	nodesTx, err := d.Begin()
	if err != nil {
		return err
	}
	edgesTx, err := d.Begin()
	if err != nil {
		return err
	}

	// Create each table in their corresponding transactions to speed up COPY
	if _, err := nodesTx.Exec(createNodesTable); err != nil {
		return fmt.Errorf("error truncating Nodes table: %v", err)
	} else if _, err := edgesTx.Exec(createEdgeTable); err != nil {
		return fmt.Errorf("error truncating Edges table: %v", err)
	}

	copyNode, err := nodesTx.Prepare(pq.CopyIn(
		"nodes",
		"ticket",
		"node_kind",
		"subkind",
		"text",
		"text_encoding",
		"start_offset",
		"end_offset",
		"snippet_start",
		"snippet_end",
		"other_facts_num",
		"other_facts",
	))
	if err != nil {
		return fmt.Errorf("error preparing Nodes copy: %v", err)
	}

	copyEdge, err := edgesTx.Prepare(pq.CopyIn(
		"edges",
		"source",
		"kind",
		"target",
		"ordinal",
	))
	if err != nil {
		return fmt.Errorf("error preparing Edges copy: %v", err)
	}

	var node srvpb.Node
	var nodeKind string
	var subkind, textEncoding *string
	var text *[]byte
	var startOffset, endOffset, snippetStart, snippetEnd *int64
	for e := range entries {
		if graphstore.IsNodeFact(e) {
			ticket := kytheuri.ToString(e.Source)
			if node.Ticket != "" && node.Ticket != ticket {
				nodeTicket := node.Ticket
				node.Ticket = ""
				var rec []byte
				if len(node.Fact) > 0 {
					rec, err = proto.Marshal(&node)
					if err != nil {
						return fmt.Errorf("error marshaling facts: %v", err)
					}
				}
				if text != nil && textEncoding == nil {
					textEncoding = proto.String(schema.DefaultTextEncoding)
				}
				if _, err := copyNode.Exec(
					nodeTicket,
					nodeKind,
					subkind,
					text,
					textEncoding,
					startOffset,
					endOffset,
					snippetStart,
					snippetEnd,
					len(node.Fact),
					rec,
				); err != nil {
					return fmt.Errorf("error copying node: %v", err)
				}
				node.Fact, text = node.Fact[0:0], nil
				nodeKind = ""
				subkind, textEncoding = nil, nil
				startOffset, endOffset, snippetStart, snippetEnd = nil, nil, nil, nil
			}
			if node.Ticket == "" {
				node.Ticket = ticket
			}
			switch e.FactName {
			case schema.NodeKindFact:
				nodeKind = string(e.FactValue)
			case schema.SubkindFact:
				subkind = proto.String(string(e.FactValue))
			case schema.TextFact:
				text = &e.FactValue
			case schema.TextEncodingFact:
				textEncoding = proto.String(string(e.FactValue))
			case schema.AnchorStartFact:
				n, err := strconv.ParseInt(string(e.FactValue), 10, 64)
				if err == nil {
					startOffset = proto.Int64(n)
				}
			case schema.AnchorEndFact:
				n, err := strconv.ParseInt(string(e.FactValue), 10, 64)
				if err == nil {
					endOffset = proto.Int64(n)
				}
			case schema.SnippetStartFact:
				n, err := strconv.ParseInt(string(e.FactValue), 10, 64)
				if err == nil {
					snippetStart = proto.Int64(n)
				}
			case schema.SnippetEndFact:
				n, err := strconv.ParseInt(string(e.FactValue), 10, 64)
				if err == nil {
					snippetEnd = proto.Int64(n)
				}
			default:
				node.Fact = append(node.Fact, &cpb.Fact{
					Name:  e.FactName,
					Value: e.FactValue,
				})
			}
		} else if schema.EdgeDirection(e.EdgeKind) == schema.Forward {
			kind, ordinal, _ := schema.ParseOrdinal(e.EdgeKind)
			ticket := kytheuri.ToString(e.Source)
			if _, err := copyEdge.Exec(ticket, kind, kytheuri.ToString(e.Target), ordinal); err != nil {
				return fmt.Errorf("error copying edge: %v", err)
			}
		}
	}

	if _, err := copyNode.Exec(); err != nil {
		return fmt.Errorf("error flushing nodes: %v", err)
	} else if _, err := copyEdge.Exec(); err != nil {
		return fmt.Errorf("error flushing edges: %v", err)
	}

	if err := nodesTx.Commit(); err != nil {
		return fmt.Errorf("error committing Nodes transaction: %v", err)
	} else if err := edgesTx.Commit(); err != nil {
		return fmt.Errorf("error committing Edges transaction: %v", err)
	}

	return nil
}

func (d *DB) createCrossRefsTable() error {
	if _, err := d.Exec(createCrossReferencesTable); err != nil {
		return fmt.Errorf("error creating cross-references table: %v", err)
	}

	txn, err := d.Begin()
	if err != nil {
		return fmt.Errorf("error creating transaction: %v", err)
	}

	copyXRef, err := txn.Prepare(pq.CopyIn("crossreferences", "ticket", "kind", "file_ticket", "anchor_ticket", "proto"))
	if err != nil {
		return fmt.Errorf("error preparing CrossReferences copy statement: %v", err)
	}

	rs, err := d.Query(`SELECT decor.target_ticket, decor.kind, decor.file_ticket, decor.anchor_ticket, anchor.start_offset, anchor.end_offset, anchor.snippet_start, anchor.snippet_end
    FROM Decorations decor
    JOIN Nodes anchor ON anchor.ticket = decor.anchor_ticket
    ORDER BY file_ticket;`)
	if err != nil {
		return fmt.Errorf("error creating decorations query: %v", err)
	}

	queryFile, err := d.Prepare("SELECT text, text_encoding FROM Nodes WHERE ticket = $1;")
	if err != nil {
		return fmt.Errorf("error preparing file query: %v", err)
	}

	var (
		file     srvpb.File
		raw      srvpb.RawAnchor
		norm     *xrefs.Normalizer
		lastFile string
	)
	for rs.Next() {
		var ticket, kind string
		var snippetStart, snippetEnd sql.NullInt64
		if err := rs.Scan(&ticket, &kind, &file.Ticket, &raw.Ticket, &raw.StartOffset, &raw.EndOffset, &snippetStart, &snippetEnd); err != nil {
			return fmt.Errorf("Decorations scan error: %v", err)
		}

		if snippetStart.Valid {
			raw.SnippetStart = int32(snippetStart.Int64)
		} else {
			raw.SnippetStart = 0
		}
		if snippetEnd.Valid {
			raw.SnippetEnd = int32(snippetEnd.Int64)
		} else {
			raw.SnippetEnd = 0
		}

		if lastFile != file.Ticket {
			var textEncoding sql.NullString
			if err := queryFile.QueryRow(file.Ticket).Scan(&file.Text, &textEncoding); err != nil {
				return fmt.Errorf("error looking up file: %v", err)
			}
			file.Encoding = textEncoding.String
			norm = xrefs.NewNormalizer(file.Text)
			lastFile = file.Ticket
		}

		a, err := assemble.ExpandAnchor(&raw, &file, norm, kind)
		if err != nil {
			return fmt.Errorf("error expanding anchor: %v", err)
		}

		rec, err := proto.Marshal(a2a(a, true))
		if err != nil {
			return fmt.Errorf("error marshaling anchor: %v", err)
		}

		if _, err := copyXRef.Exec(ticket, kind, file.Ticket, raw.Ticket, rec); err != nil {
			return fmt.Errorf("copy error: %v", err)
		}
	}
	if _, err := copyXRef.Exec(); err != nil {
		return fmt.Errorf("error flushing CrossReferences: %v", err)
	}
	if err := txn.Commit(); err != nil {
		return fmt.Errorf("transaction commit error: %v", err)
	}

	return nil
}

func a2a(a *srvpb.ExpandedAnchor, anchorText bool) *xpb.Anchor {
	var text string
	if anchorText {
		text = a.Text
	}
	return &xpb.Anchor{
		Ticket:       a.Ticket,
		Kind:         schema.Canonicalize(a.Kind),
		Parent:       a.Parent,
		Text:         text,
		Start:        p2p(a.Span.Start),
		End:          p2p(a.Span.End),
		Snippet:      a.Snippet,
		SnippetStart: p2p(a.SnippetSpan.Start),
		SnippetEnd:   p2p(a.SnippetSpan.End),
	}
}

func p2p(p *cpb.Point) *xpb.Location_Point {
	return &xpb.Location_Point{
		ByteOffset:   p.ByteOffset,
		LineNumber:   p.LineNumber,
		ColumnOffset: p.ColumnOffset,
	}
}

func (d *DB) createFileTree() error {
	if _, err := d.Exec(createFilesTable); err != nil {
		return fmt.Errorf("error creating files table: %v", err)
	}

	rs, err := d.Query("SELECT ticket FROM Nodes WHERE node_kind = 'file';")
	if err != nil {
		return fmt.Errorf("error creating files query: %v", err)
	}

	insert, err := d.Prepare(`INSERT INTO Files (corpus, root, path, ticket, file) VALUES ($1, $2, $3, $4, $5);`)
	if err != nil {
		return fmt.Errorf("error preparing statement: %v", err)
	}

	for rs.Next() {
		var ticket string
		if err := rs.Scan(&ticket); err != nil {
			return fmt.Errorf("scan error: %v", err)
		}

		uri, err := kytheuri.Parse(ticket)
		if err != nil {
			return fmt.Errorf("error parsing node ticket %q: %v", ticket, err)
		}

		path, _ := filepath.Split(filepath.Join("/", uri.Path))
		if _, err := insert.Exec(uri.Corpus, uri.Root, path, ticket, true); err != nil {
			return fmt.Errorf("error inserting file: %v", err)
		}

		uri.Signature, uri.Language = "", ""
		for {
			uri.Path = path
			path, _ = filepath.Split(strings.TrimSuffix(path, "/"))
			if path == "" {
				break
			}

			if _, err := insert.Exec(uri.Corpus, uri.Root, path, uri.String(), false); err != nil {
				if err, ok := err.(*pq.Error); ok && err.Code == pqUniqueViolationErrorCode {
					// Since we've found the current directory, we can stop recursively
					// adding parent directories now
					break
				}
				return fmt.Errorf("error inserting directory: %v", err)
			}
		}
	}

	return nil
}

const pqUniqueViolationErrorCode = pq.ErrorCode("23505")
