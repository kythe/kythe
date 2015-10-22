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

// Package analysis defines interfaces used to locate and analyze compilation
// units and their inputs.
//
// Package analysis contains the CompilationAnalyzer and Fetcher interfaces.
// CompilationAnalyzers are a generic interface to a process that analyzes
// apb.CompilationUnits, gathering their needed file data from a
// FileDataService.  Fetchers express the ability to read file contents from
// index files, local files, index packs, and other storage.
package analysis

import (
	"errors"
	"fmt"
	"io"
	"sync"

	"kythe.io/kythe/go/platform/delimited"

	"github.com/golang/protobuf/proto"
	"golang.org/x/net/context"

	apb "kythe.io/kythe/proto/analysis_proto"
	spb "kythe.io/kythe/proto/storage_proto"
)

// OutputFunc handles a single AnalysisOutput.
type OutputFunc func(context.Context, *apb.AnalysisOutput) error

// A CompilationAnalyzer analyzes compilations, retrieving all necessary file
// data from a FileDataService, and returns a stream arbitrary outputs.
type CompilationAnalyzer interface {
	// Analyze calls f on each analysis output resulting from the analysis of the
	// given apb.CompilationUnit.  If f returns an error, f is no longer called
	// and Analyze returns with the same error.
	Analyze(ctx context.Context, req *apb.AnalysisRequest, f OutputFunc) error
}

// EntryOutput returns an OutputFunc that unmarshals each output's value as an
// Entry and calls f on it.
func EntryOutput(f func(context.Context, *spb.Entry) error) OutputFunc {
	return func(ctx context.Context, out *apb.AnalysisOutput) error {
		var entry spb.Entry
		if err := proto.Unmarshal(out.Value, &entry); err != nil {
			return fmt.Errorf("error unmarshaling Entry from AnalysisOutput: %v", err)
		}
		return f(ctx, &entry)
	}
}

// A Fetcher provides the ability to fetch file contents from storage.
type Fetcher interface {
	// Fetch retrieves the contents of a single file.  At least one of path and
	// digest must be provided; both are preferred.  The implementation may
	// return an error if both are not set.
	Fetch(path, digest string) ([]byte, error)
}

// FileDataService implements the apb.FileDataServiceServer interface backed by
// a set of Fetchers.  A Fetcher can be added/removed dynamically.
type FileDataService struct {
	mu       sync.RWMutex
	fetchers []Fetcher
}

// AddFetcher adds the given Fetcher to the set of Fetchers used by Get.
func (s *FileDataService) AddFetcher(f Fetcher) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.fetchers = append(s.fetchers, f)
}

// RemoveFetcher removes the given Fetcher from the set of Fetchers used by Get.
func (s *FileDataService) RemoveFetcher(f Fetcher) {
	s.mu.Lock()
	defer s.mu.Unlock()
	for i := len(s.fetchers) - 1; i >= 0; i-- {
		if s.fetchers[i] == f {
			s.fetchers = append(s.fetchers[:i], s.fetchers[i+1:]...)
		}
	}
}

// Clear removes all Fetchers from use by Get.
func (s *FileDataService) Clear() {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.fetchers = nil
}

// Get implements the apb.FileDataServiceServer interface.
func (s *FileDataService) Get(req *apb.FilesRequest, srv apb.FileDataService_GetServer) error {
	for _, info := range req.Files {
		if info.Path == "" && info.Digest == "" {
			return errors.New("file request missing both path and digest")
		}
	}
	for _, info := range req.Files {
		s.mu.RLock()
		found := false
		for _, f := range s.fetchers {
			data, err := f.Fetch(info.Path, info.Digest)
			if err == nil {
				if err := srv.Send(&apb.FileData{
					Content: data,
					Info:    info,
				}); err != nil {
					s.mu.RUnlock()
					return err
				}
				found = true
				break
			}
		}
		s.mu.RUnlock()

		// If we did not find anything, report the file as missing.
		if !found {
			if err := srv.Send(&apb.FileData{
				Info:    info,
				Missing: true,
			}); err != nil {
				return err
			}
		}
	}
	return nil
}

type outputReader struct{ outs <-chan *apb.AnalysisOutput }

// OutputReader returns a delimited.Reader of each AnalysisOutput's value.
func OutputReader(outs <-chan *apb.AnalysisOutput) delimited.Reader { return &outputReader{outs} }

// Next implements part of the delimited.Reader interface.  Next does not return
// an error except io.EOF once the channel is closed or returns nil.
func (r *outputReader) Next() ([]byte, error) {
	out := <-r.outs
	if out == nil {
		return nil, io.EOF
	}
	return out.Value, nil
}

// NextProto implements part of the delimited.Reader interface.
func (r *outputReader) NextProto(pb proto.Message) error {
	rec, err := r.Next()
	if err != nil {
		return err
	}
	return proto.Unmarshal(rec, pb)
}
