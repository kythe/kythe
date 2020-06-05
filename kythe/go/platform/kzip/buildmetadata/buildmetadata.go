/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

// Package buildmetadata provides utilities for working with metadata kzips.
package buildmetadata // import "kythe.io/kythe/go/platform/kzip/buildmetadata"

import (
	"fmt"
	"time"

	kptypes "kythe.io/kythe/go/util/ptypes"

	"github.com/golang/protobuf/ptypes"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

const (
	// BuildMetadataLanguage is the language recorded in the VName for
	// BuildMetadata compilation units.
	Language = "kythe_build_metadata"
)

// CreateMetadataUnit creates a compilation unit containing a BuildMetadata with
// the given timestamp.
func CreateMetadataUnit(timestamp time.Time) (*apb.CompilationUnit, error) {
	tp, err := ptypes.TimestampProto(timestamp)
	if err != nil {
		return nil, fmt.Errorf("marshaling timestamp: %v", err)
	}
	meta := &apb.BuildMetadata{
		CommitTimestamp: tp,
	}
	det, err := kptypes.MarshalAny(meta)
	if err != nil {
		return nil, fmt.Errorf("marshaling details: %v", err)
	}
	return &apb.CompilationUnit{
		VName: &spb.VName{
			Language: Language,
		},
		Details: []*kptypes.Any{det},
	}, nil
}
