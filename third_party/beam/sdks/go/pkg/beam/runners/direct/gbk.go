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

package direct

import (
	"bytes"
	"context"
	"encoding/binary"
	"flag"
	"fmt"
	"hash/crc32"
	"io"
	"log"
	"runtime"
	"sync"

	"kythe.io/kythe/go/util/disksort"
	"kythe.io/kythe/go/util/sortutil"

	"github.com/apache/beam/sdks/go/pkg/beam/core/graph"
	"github.com/apache/beam/sdks/go/pkg/beam/core/runtime/exec"
	"golang.org/x/sync/errgroup"
)

var (
	gbkConcurrency = flag.Int("beam_direct_gbk_concurrency", runtime.GOMAXPROCS(0)/2,
		"Maximum allowed concurrency across all GBK operations")
	concurrencyGroupSem   chan struct{}
	setupConcurrencyGroup sync.Once
)

// CoGBK buffers all input and continues on FinishBundle. Use with small single-bundle data only.
type CoGBK struct {
	UID  exec.UnitID
	Edge *graph.MultiEdge
	Out  exec.Node

	keyEnc exec.ElementEncoder
	keyDec exec.ElementDecoder

	valEnc []exec.ElementEncoder
	valDec []exec.ElementDecoder

	sorters []disksort.Interface
	mu      []sync.Mutex
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
	concurrency := *gbkConcurrency
	if concurrency <= 0 {
		concurrency = 1
	}
	setupConcurrencyGroup.Do(func() { concurrencyGroupSem = make(chan struct{}, concurrency) })

	n.keyEnc = exec.MakeElementEncoder(n.Edge.Input[0].From.Coder.Components[0])
	n.keyDec = exec.MakeElementDecoder(n.Edge.Input[0].From.Coder.Components[0])

	for _, input := range n.Edge.Input {
		n.valEnc = append(n.valEnc, exec.MakeElementEncoder(input.From.Coder.Components[1]))
		n.valDec = append(n.valDec, exec.MakeElementDecoder(input.From.Coder.Components[1]))
	}

	for i := 0; i < concurrency; i++ {
		s, err := disksort.NewMergeSorter(disksort.MergeOptions{
			CompressShards: true,
			Marshaler:      iterValueMarshaler{},
			Lesser:         sortutil.LesserFunc(iterValueLess),
		})
		if err != nil {
			return fmt.Errorf("error creating MergeSorter: %v", err)
		}
		n.sorters = append(n.sorters, s)
	}
	n.mu = make([]sync.Mutex, len(n.sorters))

	return nil
}

func (n *CoGBK) StartBundle(ctx context.Context, id string, data exec.DataManager) error {
	return n.Out.StartBundle(ctx, id, data)
}

func (n *CoGBK) ProcessElement(ctx context.Context, elm exec.FullValue, _ ...exec.ReStream) error {
	index := elm.Elm.(int)             // index of Inputs
	value := elm.Elm2.(exec.FullValue) // actual KV<K,V>

	fullKey := exec.FullValue{Elm: value.Elm, Timestamp: value.Timestamp}  // strip V from KV<K,V>
	fullVal := exec.FullValue{Elm: value.Elm2, Timestamp: value.Timestamp} // strip K from KV<K,V>

	var buf bytes.Buffer
	if err := n.keyEnc.Encode(fullKey, &buf); err != nil {
		return fmt.Errorf("failed to encode key %v for CoGBK: %v", elm, err)
	}
	key := append([]byte{}, buf.Bytes()...)
	buf.Reset()
	if err := n.valEnc[index].Encode(fullVal, &buf); err != nil {
		return fmt.Errorf("failed to encode val %v for CoGBK: %v", elm, err)
	}
	val := buf.Bytes()

	shard := int(crc32.ChecksumIEEE(key)) % len(n.sorters)
	n.mu[shard].Lock()
	defer n.mu[shard].Unlock()
	return n.sorters[shard].Add(&iterValue{
		Key:   key,
		Index: index,
		Value: val,
	})
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

func (i *iterStream) Open() exec.Stream {
	// TODO(schroederc): add disksort support for reiteration
	if i.opened {
		panic("disksort CoGBK does not support ReStreams")
	}
	i.opened = true
	return i
}

func (i *iterStream) Read() (exec.FullValue, error) {
	iv, err := i.next()
	if err != nil {
		return exec.FullValue{}, err
	}
	val, err := i.n.valDec[i.idx].Decode(bytes.NewBuffer(iv.Value))
	if err != nil {
		return exec.FullValue{}, fmt.Errorf("error decoding value: %v", err)
	}
	return val, nil
}

func (i *iterStream) next() (*iterValue, error) {
	if i.prev != nil {
		var prevItems int
		for {
			_, err := i.prev.next()
			if err == io.EOF {
				break
			} else if err != nil {
				return nil, fmt.Errorf("previous iterator error: %v", err)
			}
			prevItems++
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
	g, ctx := errgroup.WithContext(ctx)
	keyCounts := make([]int, len(n.sorters))
	for i := 0; i < len(n.sorters); i++ {
		i := i
		g.Go(func() error {
			concurrencyGroupSem <- struct{}{}
			defer func() { <-concurrencyGroupSem }()

			iter, err := n.sorters[i].Iterator()
			if err != nil {
				return fmt.Errorf("error creating disksort.Iterator: %v", err)
			}
			next, iterErr := iter.Next()
			for {
				if iterErr == io.EOF {
					if err := iter.Close(); err != nil {
						return fmt.Errorf("error closing disksort.Iterator: %v", err)
					}
					return nil
				} else if iterErr != nil {
					return fmt.Errorf("error reading disksort.Iterator: %v", iterErr)
				}

				iv := next.(*iterValue)
				next = nil
				keyCounts[i]++

				fullKey, err := n.keyDec.Decode(bytes.NewBuffer(iv.Key))
				if err != nil {
					return fmt.Errorf("error decoding key %q: %v", string(iv.Key), err)
				}

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
		})
	}
	if err := g.Wait(); err != nil {
		return err
	}
	var totalKeys int
	for _, cnt := range keyCounts {
		totalKeys += cnt
	}
	log.Printf("CoGBK Σ%v = %d", keyCounts, totalKeys)
	return n.Out.FinishBundle(ctx)
}

func (n *CoGBK) Down(ctx context.Context) error {
	return nil
}

func (n *CoGBK) String() string {
	return fmt.Sprintf("CoGBK. Out:%v", n.Out.ID())
}
