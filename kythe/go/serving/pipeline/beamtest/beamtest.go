/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

// Package beamtest contains utilities to test Apache Beam pipelines.
package beamtest // import "kythe.io/kythe/go/serving/pipeline/beamtest"

import (
	"fmt"
	"reflect"
	"testing"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/core/runtime"
	"github.com/apache/beam/sdks/go/pkg/beam/core/util/reflectx"
	"github.com/golang/protobuf/proto"
)

// CheckRegistrations returns an error if p uses any non-registered types.
func CheckRegistrations(t *testing.T, p *beam.Pipeline) {
	edges, nodes, err := p.Build()
	if err != nil {
		t.Fatalf("invalid pipeline: %v", err)
	}
	for _, n := range nodes {
		if err := checkFullType(n.Type()); err != nil {
			t.Error(n)
		}
	}
	for _, e := range edges {
		if fn := e.DoFn; fn != nil {
			if fn.Recv != nil {
				if err := checkFnType(reflect.TypeOf(fn.Recv)); err != nil {
					t.Error(err)
				}
			} else if _, err := runtime.ResolveFunction(fn.Name(), fn.Fn.Fn.Type()); err != nil {
				t.Errorf("error resolving function: %v", err)
			}
		}
	}
}

var protoMessageType = reflect.TypeOf((*proto.Message)(nil)).Elem()

func checkType(t reflect.Type) error {
	key, keyValid := runtime.TypeKey(reflectx.SkipPtr(t))
	if !keyValid {
		return nil
	} else if _, ok := runtime.LookupType(key); !ok && t.Implements(protoMessageType) {
		return fmt.Errorf("unregistered proto.Message type: %v", t)
	}
	return nil
}

func checkFnType(t reflect.Type) error {
	key, keyValid := runtime.TypeKey(reflectx.SkipPtr(t))
	if !keyValid {
		return nil
	} else if _, ok := runtime.LookupType(key); !ok {
		return fmt.Errorf("unregistered function type: %v", t)
	}
	return nil
}

func checkFullType(t beam.FullType) error {
	if err := checkType(t.Type()); err != nil {
		return err
	}
	for _, c := range t.Components() {
		if err := checkFullType(c); err != nil {
			return err
		}
	}
	return nil
}
