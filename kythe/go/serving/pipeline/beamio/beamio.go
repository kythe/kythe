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

// Package beamio provides Beam transformations for common IO patterns.
package beamio // import "kythe.io/kythe/go/serving/pipeline/beamio"

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"reflect"

	"github.com/apache/beam/sdks/go/pkg/beam"
)

func init() {
	beam.RegisterType(reflect.TypeOf((*encodeKeyValue)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*KeyValue)(nil)).Elem())
}

// EncodeKeyValues encodes each PCollection of KVs into encoded KeyValues and
// flattens all entries into a single PCollection.
func EncodeKeyValues(s beam.Scope, tables ...beam.PCollection) beam.PCollection {
	var encodings []beam.PCollection
	for _, table := range tables {
		t := table.Type()
		encoded := beam.ParDo(s, &encodeKeyValue{
			KeyType:   beam.EncodedType{t.Components()[0].Type()},
			ValueType: beam.EncodedType{t.Components()[1].Type()},
		}, table)
		encodings = append(encodings, encoded)
	}
	return beam.Flatten(s, encodings...)
}

type encodeKeyValue struct{ KeyType, ValueType beam.EncodedType }

func (e *encodeKeyValue) ProcessElement(key beam.T, val beam.U) (KeyValue, error) {
	keyEnc := beam.NewElementEncoder(e.KeyType.T)
	var keyBuf bytes.Buffer
	if err := keyEnc.Encode(key, &keyBuf); err != nil {
		return KeyValue{}, err
	} else if _, err := binary.ReadUvarint(&keyBuf); err != nil {
		return KeyValue{}, fmt.Errorf("error removing varint prefix from key encoding: %v", err)
	}
	valEnc := beam.NewElementEncoder(e.ValueType.T)
	var valBuf bytes.Buffer
	if err := valEnc.Encode(val, &valBuf); err != nil {
		return KeyValue{}, err
	} else if _, err := binary.ReadUvarint(&valBuf); err != nil {
		return KeyValue{}, fmt.Errorf("error removing varint prefix from value encoding: %v", err)
	}
	return KeyValue{Key: keyBuf.Bytes(), Value: valBuf.Bytes()}, nil
}

// A KeyValue is a concrete form of a Beam KV.
type KeyValue struct {
	Key   []byte `json:"k"`
	Value []byte `json:"v"`
}
