/*
 * Copyright 2018 Google Inc. All rights reserved.
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

package pipeline

import (
	"kythe.io/kythe/go/serving/pipeline/nodes"

	"github.com/apache/beam/sdks/go/pkg/beam"
)

// KytheBeam controls the lifetime and generation of PCollections in the Kythe
// pipeline.
type KytheBeam struct {
	s beam.Scope

	nodes beam.PCollection
}

// FromEntries creates a KytheBeam pipeline from an input collection of
// *spb.Entry messages.
func FromEntries(s beam.Scope, entries beam.PCollection) *KytheBeam {
	return &KytheBeam{s: s, nodes: nodes.FromEntries(s, entries)}
}

// Nodes returns all *ppb.Nodes from the Kythe input graph.
func (k *KytheBeam) Nodes() beam.PCollection { return k.nodes }
