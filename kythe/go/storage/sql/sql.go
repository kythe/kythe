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

// Package sql implements a GraphStore using a SQL database backend.
package sql

import (
	"database/sql"
	"errors"
	"fmt"
	"log"
	"strings"

	spb "kythe/proto/storage_proto"

	_ "github.com/mattn/go-sqlite3" // register the "sqlite3" driver
)

// SQLite3 is the standard database/sql driver name for sqlite3.
const SQLite3 = "sqlite3"

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

// DB is a wrapper around a sql.DB that implements storage.GraphStore
type DB struct {
	*sql.DB
	writeStmt *sql.Stmt
}

// OpenGraphStore returns a GraphStore backed by a SQL database at the given
// filepath.
func OpenGraphStore(driverName, dataSourceName string) (*DB, error) {
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
	d.writeStmt, err = db.Prepare("INSERT OR REPLACE INTO kythe (" + columns + ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)")
	if err != nil {
		return nil, fmt.Errorf("error preparing write statement: %v", err)
	}
	return d, nil
}

// Read implements part of the GraphStore interface.
func (db *DB) Read(req *spb.ReadRequest, stream chan<- *spb.Entry) (err error) {
	if req.Source == nil {
		return errors.New("invalid ReadRequest: missing Source")
	}

	var rows *sql.Rows
	const baseQuery = "SELECT " + columns + " FROM " + tableName + " WHERE source_signature = ? AND source_corpus = ? AND source_root = ? AND source_language = ? AND source_path = ? "
	if req.GetEdgeKind() == "*" {
		rows, err = db.Query(baseQuery+orderByClause, req.GetSource().GetSignature(), req.GetSource().GetCorpus(), req.GetSource().GetRoot(), req.GetSource().GetLanguage(), req.GetSource().GetPath())
	} else {
		rows, err = db.Query(baseQuery+"AND kind = ? "+orderByClause, req.GetSource().GetSignature(), req.GetSource().GetCorpus(), req.GetSource().GetRoot(), req.GetSource().GetLanguage(), req.GetSource().GetPath(), req.GetEdgeKind())
	}
	if err != nil {
		return fmt.Errorf("sql select error: %v", err)
	}
	return scanEntries(rows, stream)
}

var likeEscaper = strings.NewReplacer("%", "\t%", "_", "\t_")

// Scan implements part of the GraphStore interface.
func (db *DB) Scan(req *spb.ScanRequest, stream chan<- *spb.Entry) (err error) {
	var rows *sql.Rows
	factPrefix := likeEscaper.Replace(req.GetFactPrefix()) + "%"
	if req.Target != nil {
		if req.GetEdgeKind() == "" {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+` WHERE fact LIKE ? ESCAPE '\t' AND source_signature = ? AND source_corpus = ? AND source_root = ? AND source_language = ? AND source_path = ? `+orderByClause, factPrefix, req.GetTarget().GetSignature(), req.GetTarget().GetCorpus(), req.GetTarget().GetRoot(), req.GetTarget().GetLanguage(), req.GetTarget().GetPath())
		} else {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+` WHERE fact LIKE ? ESCAPE '\t' AND source_signature = ? AND source_corpus = ? AND source_root = ? AND source_language = ? AND source_path = ? AND kind = ? `+orderByClause, factPrefix, req.GetTarget().GetSignature(), req.GetTarget().GetCorpus(), req.GetTarget().GetRoot(), req.GetTarget().GetLanguage(), req.GetTarget().GetPath(), req.GetEdgeKind())
		}
	} else {
		if req.GetEdgeKind() == "" {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+" WHERE fact LIKE ? ESCAPE '\t' "+orderByClause, factPrefix)
		} else {
			rows, err = db.Query("SELECT "+columns+" FROM "+tableName+` WHERE fact LIKE ? ESCAPE '\t' AND kind = ? `+orderByClause, factPrefix, req.GetEdgeKind())
		}
	}
	if err != nil {
		return err
	}
	return scanEntries(rows, stream)
}

// Write implements part of the GraphStore interface.
func (db *DB) Write(req *spb.WriteRequest) error {
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
		_, err := writeStmt.Exec(
			req.GetSource().GetSignature(), req.GetSource().GetCorpus(), req.GetSource().GetRoot(), req.GetSource().GetPath(), req.GetSource().GetLanguage(),
			update.GetEdgeKind(),
			update.GetFactName(),
			update.GetTarget().GetSignature(), update.GetTarget().GetCorpus(), update.GetTarget().GetRoot(), update.GetTarget().GetPath(), update.GetTarget().GetLanguage(),
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

func scanEntries(rows *sql.Rows, stream chan<- *spb.Entry) error {
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
		if entry.GetEdgeKind() == "" {
			entry.EdgeKind = nil
			entry.Target = nil
		}
		stream <- entry
	}
	return rows.Close()
}
