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

// Package pq implements the Kythe serving api.Interface using a Postgres
// database backend.  The database can be populated using the DB.CopyGraphStore
// method.
package pq

import (
	"database/sql"
	"fmt"

	"kythe.io/kythe/go/serving/api"
)

// DB implements the api.Interface
type DB struct {
	*sql.DB

	selectText, selectCorpusRoots, selectDirectory *sql.Stmt
}

var _ api.Interface = new(DB)

// Open returns an api.Interface wrapper around the connection returned by
// sql.Open("postgres", spec).
func Open(spec string) (*DB, error) {
	db, err := sql.Open("postgres", spec)
	if err != nil {
		return nil, err
	}
	if err := db.Ping(); err != nil {
		db.Close()
		return nil, fmt.Errorf("error pinging db: %v", err)
	}
	return &DB{DB: db}, nil
}
