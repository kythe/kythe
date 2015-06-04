/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Package sql implements a graphstore.Service using a SQL database backend.  A
// specialized implementation for an xrefs.Service is also available.
package sql

import (
	"database/sql"
	"errors"
	"fmt"
	"io"
	"log"
	"strings"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"

	"golang.org/x/net/context"

	spb "kythe.io/kythe/proto/storage_proto"

	_ "github.com/mattn/go-sqlite3" // register the "sqlite3" driver
)

// SQLite3 is the standard database/sql driver name for sqlite3.
const SQLite3 = "sqlite3"

func init() {
	gsutil.Register(SQLite3, func(spec string) (graphstore.Service, error) { return Open(SQLite3, spec) })
}

const (
	tableName     = "kythe"
	uniqueColumns = "source_signature, source_corpus, source_root, source_path, source_language, kind, fact, target_signature, target_corpus, target_root, target_path, target_language"
	columns       = uniqueColumns + ", value"
	orderByClause = "ORDER BY " + columns

	initStmt = `PRAGMA synchronous = OFF;
PRAGMA journal_mode = WAL;
PRAGMA encoding = "UTF-8";
PRAGMA case_sensitive_like = true;
CREATE TABLE IF NOT EXISTS ` + tableName + " (" + columns + ", UNIQUE(" + uniqueColumns + "))"
)

// Prepared statements
const (
	writeStmt = "INSERT OR REPLACE INTO kythe (" + columns + ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)"

	anchorEdgesStmt = `
SELECT
  target_signature,
  target_corpus,
  target_root,
  target_path,
  target_language,
  kind
FROM kythe
WHERE kind != ""
  AND kind != ?
  AND kind NOT LIKE "!%" ESCAPE '!'
  AND source_signature = ?
  AND source_corpus = ?
  AND source_root = ?
  AND source_path = ?
  AND source_language = ?`

	edgesStmt = `
SELECT
  target_signature,
  target_corpus,
  target_root,
  target_path,
  target_language,
  kind
FROM kythe
WHERE source_signature = ?
  AND source_corpus = ?
  AND source_root = ?
  AND source_path = ?
  AND source_language = ?
  AND kind != ""`

	revEdgesStmt = `
SELECT
  source_signature,
  source_corpus,
  source_root,
  source_path,
  source_language,
  kind
FROM kythe
WHERE target_signature = ?
  AND target_corpus = ?
  AND target_root = ?
  AND target_path = ?
  AND target_language = ?
  AND kind != ""`

	nodeFactsStmt = `
SELECT fact, value
FROM kythe
WHERE source_signature = ?
  AND source_corpus = ?
  AND source_root = ?
  AND source_language = ?
  AND source_path = ?
  AND kind = ""`
)

// DB is a wrapper around a sql.DB that implements graphstore.Service
type DB struct {
	*sql.DB

	writeStmt       *sql.Stmt
	nodeFactsStmt   *sql.Stmt
	edgesStmt       *sql.Stmt
	revEdgesStmt    *sql.Stmt
	anchorEdgesStmt *sql.Stmt
}

// Open returns an opened SQL database at the given path that can be used as a GraphStore and
// xrefs.Service.
func Open(driverName, dataSourceName string) (*DB, error) {
	db, err := sql.Open(driverName, dataSourceName)
	if err != nil {
		return nil, err
	}
	db.SetMaxOpenConns(1) // TODO(schroederc): Without this, concurrent writers will get a "database
	//                                         is locked" error from sqlite3.  Unfortunately, writing
	//                                         concurrently with a scan/read will now deadlock...
	if _, err := db.Exec(initStmt); err != nil {
		return nil, fmt.Errorf("error initializing SQL db: %v", err)
	}
	d := &DB{DB: db}
	d.writeStmt, err = db.Prepare(writeStmt)
	if err != nil {
		return nil, fmt.Errorf("error preparing write statement: %v", err)
	}
	d.anchorEdgesStmt, err = db.Prepare(anchorEdgesStmt)
	if err != nil {
		return nil, fmt.Errorf("error preparing anchor edges statement: %v", err)
	}
	d.edgesStmt, err = db.Prepare(edgesStmt)
	if err != nil {
		return nil, fmt.Errorf("error preparing edges statement: %v", err)
	}
	d.revEdgesStmt, err = db.Prepare(revEdgesStmt)
	if err != nil {
		return nil, fmt.Errorf("error preparing reverse edges statement: %v", err)
	}
	d.nodeFactsStmt, err = db.Prepare(nodeFactsStmt)
	if err != nil {
		return nil, fmt.Errorf("error preparing node facts statement: %v", err)
	}
	return d, nil
}

// Close implements part of the graphstore.Service interface.
func (db *DB) Close(ctx context.Context) error { return db.DB.Close() }

// Read implements part of the graphstore.Service interface.
func (db *DB) Read(ctx context.Context, req *spb.ReadRequest, f graphstore.EntryFunc) (err error) {
	if req.GetSource() == nil {
		return errors.New("invalid ReadRequest: missing Source")
	}

	var rows *sql.Rows
	const baseQuery = "SELECT " + columns + " FROM " + tableName + " WHERE source_signature = ? AND source_corpus = ? AND source_root = ? AND source_language = ? AND source_path = ? "
	if req.EdgeKind == "*" {
		rows, err = db.Query(baseQuery+orderByClause, req.Source.Signature, req.Source.Corpus, req.Source.Root, req.Source.Language, req.Source.Path)
	} else {
		rows, err = db.Query(baseQuery+"AND kind = ? "+orderByClause, req.Source.Signature, req.Source.Corpus, req.Source.Root, req.Source.Language, req.Source.Path, req.EdgeKind)
	}
	if err != nil {
		return fmt.Errorf("sql select error: %v", err)
	}
	return scanEntries(rows, f)
}

var likeEscaper = strings.NewReplacer("%", "\t%", "_", "\t_")

// Scan implements part of the graphstore.Service interface.
//
// TODO(fromberger): Maybe use prepared statements here.
func (db *DB) Scan(ctx context.Context, req *spb.ScanRequest, f graphstore.EntryFunc) (err error) {
	var rows *sql.Rows
	factPrefix := likeEscaper.Replace(req.FactPrefix) + "%"
	if req.GetTarget() != nil {
		if req.EdgeKind == "" {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+` WHERE fact LIKE ? ESCAPE '\t' AND source_signature = ? AND source_corpus = ? AND source_root = ? AND source_language = ? AND source_path = ? `+orderByClause, factPrefix, req.Target.Signature, req.Target.Corpus, req.Target.Root, req.Target.Language, req.Target.Path)
		} else {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+` WHERE fact LIKE ? ESCAPE '\t' AND source_signature = ? AND source_corpus = ? AND source_root = ? AND source_language = ? AND source_path = ? AND kind = ? `+orderByClause, factPrefix, req.Target.Signature, req.Target.Corpus, req.Target.Root, req.Target.Language, req.Target.Path, req.EdgeKind)
		}
	} else {
		if req.EdgeKind == "" {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+" WHERE fact LIKE ? ESCAPE '\t' "+orderByClause, factPrefix)
		} else {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+` WHERE fact LIKE ? ESCAPE '\t' AND kind = ? `+orderByClause, factPrefix, req.EdgeKind)
		}
	}
	if err != nil {
		return err
	}
	return scanEntries(rows, f)
}

var emptyVName = new(spb.VName)

// Write implements part of the graphstore.Service interface.
func (db *DB) Write(ctx context.Context, req *spb.WriteRequest) error {
	if req.GetSource() == nil {
		return fmt.Errorf("missing Source in WriteRequest: {%v}", req)
	}
	tx, err := db.Begin()
	if err != nil {
		return fmt.Errorf("sql transaction begin error: %v", err)
	}
	writeStmt := tx.Stmt(db.writeStmt)
	defer func() {
		if err := writeStmt.Close(); err != nil {
			log.Printf("error closing SQL Stmt: %v", err)
		}
	}()
	for _, update := range req.Update {
		if update.Target == nil {
			update.Target = emptyVName
		}
		_, err := writeStmt.Exec(
			req.Source.Signature, req.Source.Corpus, req.Source.Root, req.Source.Path, req.Source.Language,
			update.EdgeKind,
			update.FactName,
			update.Target.Signature, update.Target.Corpus, update.Target.Root, update.Target.Path, update.Target.Language,
			update.FactValue)
		if err != nil {
			tx.Rollback()
			return fmt.Errorf("sql insertion error: %v", err)
		}
	}
	if err := tx.Commit(); err != nil {
		return fmt.Errorf("sql commit error: %v", err)
	}
	return nil
}

func scanEntries(rows *sql.Rows, f graphstore.EntryFunc) error {
	for rows.Next() {
		entry := &spb.Entry{
			Source: &spb.VName{},
			Target: &spb.VName{},
		}
		err := rows.Scan(
			&entry.Source.Signature,
			&entry.Source.Corpus,
			&entry.Source.Root,
			&entry.Source.Path,
			&entry.Source.Language,
			&entry.EdgeKind,
			&entry.FactName,
			&entry.Target.Signature,
			&entry.Target.Corpus,
			&entry.Target.Root,
			&entry.Target.Path,
			&entry.Target.Language,
			&entry.FactValue)
		if err != nil {
			rows.Close() // ignore errors
			return err
		}
		if entry.EdgeKind == "" {
			entry.Target = nil
		}
		if err := f(entry); err == io.EOF {
			rows.Close()
			return nil
		} else if err != nil {
			rows.Close()
			return err
		}
	}
	return rows.Close()
}
