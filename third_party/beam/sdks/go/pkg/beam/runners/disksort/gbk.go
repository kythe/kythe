// Licensed to the Apache Software Foundation (ASF) under one or more
// contributor license agreements.  See the NOTICE file distributed with
// this work for additional information regarding copyright ownership.
// The ASF licenses this file to You under the Apache License, Version 2.0
// (the "License"); you may not use this file except in compliance with
// the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package disksort

import (
	"bytes"
	"context"
	"encoding/binary"
	"fmt"
	"io"
	"log"

	"kythe.io/kythe/go/util/disksort"
	"kythe.io/kythe/go/util/sortutil"

	"github.com/apache/beam/sdks/go/pkg/beam/core/graph"
	"github.com/apache/beam/sdks/go/pkg/beam/core/runtime/exec"
	"github.com/apache/beam/sdks/go/pkg/beam/core/typex"
)

// CoGBK buffers all input and continues on FinishBundle. Use with small single-bundle data only.
type CoGBK struct {
	UID  exec.UnitID
	Edge *graph.MultiEdge
	Out  exec.Node

	wEnc   exec.WindowEncoder
	wDec   exec.WindowDecoder
	keyEnc exec.ElementEncoder
	keyDec exec.ElementDecoder

	valEnc []exec.ElementEncoder
	valDec []exec.ElementDecoder

	sorter disksort.Interface
}

func (n *CoGBK) ID() exec.UnitID {
	return n.UID
}

func iterValueLess(a, b interface{}) bool {
	av := a.(*iterValue)
	bv := b.(*iterValue)
	c := bytes.Compare(av.Key, bv.Key)
	if c == 0 {
		return av.Index < bv.Index
	}
	return c < 0
}

func (n *CoGBK) Up(ctx context.Context) error {
	n.wEnc = exec.MakeWindowEncoder(n.Edge.Input[0].From.WindowingStrategy().Fn.Coder())
	n.wDec = exec.MakeWindowDecoder(n.Edge.Input[0].From.WindowingStrategy().Fn.Coder())
	n.keyEnc = exec.MakeElementEncoder(n.Edge.Input[0].From.Coder.Components[0])
	n.keyDec = exec.MakeElementDecoder(n.Edge.Input[0].From.Coder.Components[0])

	for _, input := range n.Edge.Input {
		n.valEnc = append(n.valEnc, exec.MakeElementEncoder(input.From.Coder.Components[1]))
		n.valDec = append(n.valDec, exec.MakeElementDecoder(input.From.Coder.Components[1]))
	}

	s, err := disksort.NewMergeSorter(disksort.MergeOptions{
		Name: fmt.Sprintf("beam.%s.%s", n.Edge.Name(), n.Edge.Scope()),

		MaxInMemory:      1024 * 1024,
		MaxBytesInMemory: 1024 * 1024 * 256,
		CompressShards:   true,
		Marshaler:        iterValueMarshaler{},
		Lesser:           sortutil.LesserFunc(iterValueLess),
	})
	if err != nil {
		return fmt.Errorf("error creating MergeSorter: %v", err)
	}
	n.sorter = s

	return nil
}

func (n *CoGBK) StartBundle(ctx context.Context, id string, data exec.DataContext) error {
	return n.Out.StartBundle(ctx, id, data)
}

func (n *CoGBK) ProcessElement(ctx context.Context, elm *exec.FullValue, _ ...exec.ReStream) error {
	index := elm.Elm.(int)              // index of Inputs
	value := elm.Elm2.(*exec.FullValue) // actual KV<K,V>

	fullVal := &exec.FullValue{Elm: value.Elm2, Timestamp: value.Timestamp} // strip K from KV<K,V>
	var buf bytes.Buffer
	if err := n.valEnc[index].Encode(fullVal, &buf); err != nil {
		return fmt.Errorf("failed to encode val %v for CoGBK: %v", elm, err)
	}
	val := append([]byte{}, buf.Bytes()...)

	// Encode KV per window
	for _, w := range elm.Windows {
		ws := []typex.Window{w}
		fullKey := &exec.FullValue{Elm: value.Elm, Timestamp: value.Timestamp, Windows: ws} // strip V from KV<K,V>

		buf.Reset()
		if err := n.keyEnc.Encode(fullKey, &buf); err != nil {
			return fmt.Errorf("failed to encode key %v for CoGBK: %v", elm, err)
		}
		if err := n.wEnc.Encode(ws, &buf); err != nil {
			return fmt.Errorf("failed to encode window %v for CoGBK: %v", w, err)
		}
		key := append([]byte{}, buf.Bytes()...)

		if err := n.sorter.Add(&iterValue{
			Key:   key,
			Index: index,
			Value: val,
		}); err != nil {
			return err
		}
	}
	return nil
}

type iterStream struct {
	n *CoGBK

	iter disksort.Iterator

	key []byte
	idx int

	buffer *iterValue // first element of this/next shard
	prev   *iterStream

	opened bool
}

func (i *iterStream) Open() (exec.Stream, error) {
	// TODO(schroederc): add disksort support for reiteration
	if i.opened {
		return nil, fmt.Errorf("disksort CoGBK does not support ReStreams")
	}
	i.opened = true
	return i, nil
}

func (i *iterStream) Read() (*exec.FullValue, error) {
	iv, err := i.next()
	if err != nil {
		return nil, err
	}
	val, err := i.n.valDec[i.idx].Decode(bytes.NewBuffer(iv.Value))
	if err != nil {
		return nil, fmt.Errorf("error decoding value: %v", err)
	}
	return val, nil
}

func (i *iterStream) next() (*iterValue, error) {
	if i.prev != nil {
		for {
			_, err := i.prev.next()
			if err == io.EOF {
				break
			} else if err != nil {
				return nil, fmt.Errorf("previous iterator error: %v", err)
			}
		}
		i.buffer = i.prev.buffer
		i.prev = nil
	}

	if i.iter == nil {
		return nil, io.EOF
	} else if i.buffer != nil {
		if bytes.Equal(i.buffer.Key, i.key) && i.buffer.Index == i.idx {
			iv := i.buffer
			i.buffer = nil
			return iv, nil
		}
		return nil, io.EOF
	}

	next, err := i.iter.Next()
	if err == io.EOF {
		return nil, io.EOF
	} else if err != nil {
		return nil, fmt.Errorf("error reading CoGBK[%v] element for key %q: %v", i.n.ID(), string(i.key), err)
	}
	iv := next.(*iterValue)
	if bytes.Equal(iv.Key, i.key) && iv.Index == i.idx {
		return iv, nil
	}
	i.buffer = iv
	return nil, io.EOF
}

func (i *iterStream) Close() error {
	i.iter = nil
	return nil
}

type iterValue struct {
	Key   []byte
	Index int
	Value []byte
}

func (v *iterValue) Size() int { return len(v.Key) + len(v.Value) + 3*binary.MaxVarintLen64 }

func (v *iterValue) String() string {
	return fmt.Sprintf("key=%q index=%d len(val)=%d", string(v.Key), v.Index, len(v.Value))
}

type iterValueMarshaler struct{}

func (iterValueMarshaler) Marshal(v interface{}) ([]byte, error) {
	iv := v.(*iterValue)
	buf := make([]byte, binary.MaxVarintLen64+len(iv.Key)+binary.MaxVarintLen64+binary.MaxVarintLen64+len(iv.Value))
	i := binary.PutVarint(buf, int64(len(iv.Key)))
	copy(buf[i:], iv.Key)
	i += len(iv.Key)
	i += binary.PutVarint(buf[i:], int64(iv.Index))
	i += binary.PutVarint(buf[i:], int64(len(iv.Value)))
	copy(buf[i:], iv.Value)
	i += len(iv.Value)
	return buf[:i], nil
}
func (iterValueMarshaler) Unmarshal(rec []byte) (interface{}, error) {
	var iv iterValue
	r := bytes.NewReader(rec)
	keySize, err := binary.ReadVarint(r)
	if err != nil {
		return nil, fmt.Errorf("error reading key size: %v", err)
	}
	iv.Key = make([]byte, int(keySize))
	if n, err := r.Read(iv.Key); err != nil || n != int(keySize) {
		return nil, fmt.Errorf("error reading key %d!=%d: %v", n, keySize, err)
	}
	idx, err := binary.ReadVarint(r)
	if err != nil {
		return nil, fmt.Errorf("error reading index: %v", err)
	}
	iv.Index = int(idx)
	valueSize, err := binary.ReadVarint(r)
	if err != nil {
		return nil, fmt.Errorf("error reading value size: %v", err)
	}
	iv.Value = make([]byte, int(valueSize))
	if n, err := r.Read(iv.Value); err != nil || n != int(valueSize) {
		return nil, fmt.Errorf("error reading value %d!=%d: %v", n, valueSize, err)
	}
	return &iv, nil
}

func (n *CoGBK) FinishBundle(ctx context.Context) error {
	iter, err := n.sorter.Iterator()
	if err != nil {
		return fmt.Errorf("error creating disksort.Iterator: %v", err)
	}

	var totalKeys int
	next, iterErr := iter.Next()
	for {
		if iterErr == io.EOF {
			if err := iter.Close(); err != nil {
				return fmt.Errorf("error closing disksort.Iterator: %v", err)
			}
			if *verbose {
				log.Printf("CoGBK = %d", totalKeys)
			}
			return n.Out.FinishBundle(ctx)
		} else if iterErr != nil {
			return fmt.Errorf("error reading disksort.Iterator: %v", iterErr)
		}

		iv := next.(*iterValue)
		next = nil
		totalKeys++

		buf := bytes.NewBuffer(iv.Key)
		fullKey, err := n.keyDec.Decode(buf)
		if err != nil {
			return fmt.Errorf("error decoding key %q: %v", string(iv.Key), err)
		}
		ws, err := n.wDec.Decode(buf)
		if err != nil {
			return fmt.Errorf("error decoding key window %q: %v", string(iv.Key), err)
		}
		fullKey.Windows = ws

		values := make([]exec.ReStream, len(n.Edge.Input))
		for i := 0; i < len(values); i++ {
			is := &iterStream{n: n, key: iv.Key, idx: i, iter: iter}
			if i == iv.Index {
				is.buffer = iv
			} else if i < iv.Index {
				is.iter = nil // already closed
			} else {
				is.prev = values[i-1].(*iterStream)
			}
			values[i] = is
		}
		iv = nil

		if err := n.Out.ProcessElement(ctx, fullKey, values...); err != nil {
			return err
		}

		// Advance to next key
		last := values[len(values)-1].(*iterStream)
		for {
			next, iterErr = last.next()
			if iterErr == io.EOF {
				if last.buffer != nil {
					next = last.buffer
					iterErr = nil
				}
				break
			} else if iterErr != nil {
				break
			}
		}
	}
}

func (n *CoGBK) Down(ctx context.Context) error {
	return nil
}

func (n *CoGBK) String() string {
	return fmt.Sprintf("CoGBK. Out:%v", n.Out.ID())
}
