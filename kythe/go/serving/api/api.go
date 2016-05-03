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

// Package api provides a union of the filetree and xrefs interfaces
// and a command-line flag parser.
package api

import (
	"flag"
	"fmt"
	"io"
	"os"
	"strings"

	"kythe.io/kythe/go/services/filetree"
	"kythe.io/kythe/go/services/xrefs"
	ftsrv "kythe.io/kythe/go/serving/filetree"
	xsrv "kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/storage/leveldb"
	"kythe.io/kythe/go/storage/table"

	"golang.org/x/net/context"
	"google.golang.org/grpc"

	ftpb "kythe.io/kythe/proto/filetree_proto"
	xpb "kythe.io/kythe/proto/xref_proto"
)

// Interface is a union of the xrefs and filetree interfaces.
type Interface interface {
	io.Closer
	xrefs.Service
	filetree.Service
}

const (
	// CommonDefault is the common Kythe default API specification for Flag
	CommonDefault = "https://xrefs-dot-kythe-repo.appspot.com"

	// CommonFlagUsage is the common Kythe usage description used for Flag
	CommonFlagUsage = "Backing API specification (e.g. JSON HTTP server: https://xrefs-dot-kythe-repo.appspot.com or GRPC server: localhost:1003 or local serving table path: /var/kythe_serving)"
)

// Flag defines an api Interface flag with specified name, default value, and
// usage description.  The return value is the address of an Interface variable
// that stores the value of the flag.
func Flag(name, value, usage string) *Interface {
	val := &apiFlag{}
	val.Set(value)
	flag.Var(val, name, usage)
	return &val.api
}

// ParseSpec parses the given specification and returns an opened handle to an
// API Interface.  The following formats are currently supported:
//   - http:// URL pointed at a JSON web API
//   - https:// URL pointed at a JSON web API
//   - host:port pointed at a GRPC API
//   - local path to a LevelDB serving table
func ParseSpec(apiSpec string) (Interface, error) {
	api := &apiCloser{}
	if strings.HasPrefix(apiSpec, "http://") || strings.HasPrefix(apiSpec, "https://") {
		api.xs = xrefs.WebClient(apiSpec)
		api.ft = filetree.WebClient(apiSpec)
	} else if _, err := os.Stat(apiSpec); err == nil {
		db, err := leveldb.Open(apiSpec, nil)
		if err != nil {
			return nil, fmt.Errorf("error opening local DB at %q: %v", apiSpec, err)
		}
		api.closer = func() error { return db.Close() }

		tbl := table.ProtoBatchParallel{&table.KVProto{db}}
		api.xs = xsrv.NewCombinedTable(tbl)
		api.ft = &ftsrv.Table{tbl, true}
	} else {
		conn, err := grpc.Dial(apiSpec)
		if err != nil {
			return nil, fmt.Errorf("error connecting to remote API %q: %v", apiSpec, err)
		}
		api.closer = func() error { conn.Close(); return nil }

		api.xs = xrefs.GRPC(xpb.NewXRefServiceClient(conn))
		api.ft = filetree.GRPC(ftpb.NewFileTreeServiceClient(conn))
	}
	return api, nil
}

type apiFlag struct {
	spec string
	api  Interface
}

// Set implements part of the flag.Value interface.
func (f *apiFlag) Set(spec string) error {
	api, err := ParseSpec(spec)
	if err != nil {
		return err
	}
	f.spec = spec
	f.api = api
	return nil
}

// String implements part of the flag.Value interface.
func (f *apiFlag) String() string { return f.spec }

// apiCloser implements Interface
type apiCloser struct {
	xs xrefs.Service
	ft filetree.Service

	closer func() error
}

// Close implements the io.Closer interface.
func (api apiCloser) Close() error {
	if api.closer != nil {
		return api.closer()
	}
	return nil
}

// Nodes implements part of the xrefs Service interface.
func (api apiCloser) Nodes(ctx context.Context, req *xpb.NodesRequest) (*xpb.NodesReply, error) {
	return api.xs.Nodes(ctx, req)
}

// Edges implements part of the xrefs Service interface.
func (api apiCloser) Edges(ctx context.Context, req *xpb.EdgesRequest) (*xpb.EdgesReply, error) {
	return api.xs.Edges(ctx, req)
}

// Decorations implements part of the xrefs Service interface.
func (api apiCloser) Decorations(ctx context.Context, req *xpb.DecorationsRequest) (*xpb.DecorationsReply, error) {
	return api.xs.Decorations(ctx, req)
}

// CrossReferences implements part of the xrefs Service interface.
func (api apiCloser) CrossReferences(ctx context.Context, req *xpb.CrossReferencesRequest) (*xpb.CrossReferencesReply, error) {
	return api.xs.CrossReferences(ctx, req)
}

// Callers implements part of the xrefs Service interface.
func (api apiCloser) Callers(ctx context.Context, req *xpb.CallersRequest) (*xpb.CallersReply, error) {
	return api.xs.Callers(ctx, req)
}

// Documentation implements part of the xrefs Service interface.
func (api apiCloser) Documentation(ctx context.Context, req *xpb.DocumentationRequest) (*xpb.DocumentationReply, error) {
	return api.xs.Documentation(ctx, req)
}

// Directory implements part of the filetree Service interface.
func (api apiCloser) Directory(ctx context.Context, req *ftpb.DirectoryRequest) (*ftpb.DirectoryReply, error) {
	return api.ft.Directory(ctx, req)
}

// CorpusRoots implements part of the filetree Service interface.
func (api apiCloser) CorpusRoots(ctx context.Context, req *ftpb.CorpusRootsRequest) (*ftpb.CorpusRootsReply, error) {
	return api.ft.CorpusRoots(ctx, req)
}
