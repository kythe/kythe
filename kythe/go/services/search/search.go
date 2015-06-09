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

// Package search defines a service to search for nodes from a partial VName and
// collection of facts.
package search

import (
	"log"
	"net/http"
	"time"

	"kythe.io/kythe/go/services/web"

	"golang.org/x/net/context"

	spb "kythe.io/kythe/proto/storage_proto"
)

// Service to search for nodes based on a partial VName and collection of known
// facts.
type Service interface {
	// Search returns the matching set of nodes that match the given request.
	Search(context.Context, *spb.SearchRequest) (*spb.SearchReply, error)
}

type grpcClient struct{ spb.SearchServiceClient }

// Search implements the Service interface.
func (c *grpcClient) Search(ctx context.Context, req *spb.SearchRequest) (*spb.SearchReply, error) {
	return c.SearchServiceClient.Search(ctx, req)
}

// GRPC returns a search Service backed by the given GRPC client and context.
func GRPC(c spb.SearchServiceClient) Service { return &grpcClient{c} }

type webClient struct{ addr string }

// Search implements the Service interface.
func (w *webClient) Search(ctx context.Context, q *spb.SearchRequest) (*spb.SearchReply, error) {
	var reply spb.SearchReply
	return &reply, web.Call(w.addr, "search", q, &reply)
}

// WebClient returns a search Service based on a remote web server.
func WebClient(addr string) Service {
	return &webClient{addr}
}

// RegisterHTTPHandlers registers JSON HTTP handlers with mux using the given
// search Service.  The following method with be exposed:
//
//   GET /search
//     Request: JSON encoded storage.SearchRequest
//     Response: JSON encoded storage.SearchReply
//
// Note: /search will return its response as serialized protobuf if the
// "proto" query parameter is set.
func RegisterHTTPHandlers(ctx context.Context, s Service, mux *http.ServeMux) {
	mux.HandleFunc("/search", func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		defer func() {
			log.Printf("search.Search:\t%s", time.Since(start))
		}()

		var req spb.SearchRequest
		if err := web.ReadJSONBody(r, &req); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		reply, err := s.Search(ctx, &req)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		if err := web.WriteResponse(w, r, reply); err != nil {
			log.Println(err)
		}
	})
}
