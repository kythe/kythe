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

package beamio

import (
	"bytes"
	"context"
	"encoding/binary"
	"fmt"
	"hash/crc32"
	"io"
	"log"
	"path/filepath"
	"reflect"
	"sort"
	"time"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/core/runtime/exec"
	"github.com/apache/beam/sdks/go/pkg/beam/io/filesystem"
	"github.com/syndtr/goleveldb/leveldb/comparer"
	"github.com/syndtr/goleveldb/leveldb/journal"
	"github.com/syndtr/goleveldb/leveldb/opt"
	"github.com/syndtr/goleveldb/leveldb/table"
)

func init() {
	beam.RegisterType(reflect.TypeOf((*encodeKeyValue)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*shardKeyValue)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*writeManifest)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*writeTable)(nil)).Elem())
}

// WriteLevelDB writes a set of PCollections containing KVs to a new LevelDB at
// the given path.  Each KV is serialized and stored as a single LevelDB
// key-value entry according to their enclosing PCollection's beam.Coder.  Each
// table may have different KV types.  Keys must be unique across all
// PCollections.
func WriteLevelDB(s beam.Scope, path string, numShards int, tables ...beam.PCollection) {
	filesystem.ValidateScheme(path)

	// Encode each PCollection of KVs into ([]byte, []byte) key-values (*keyValue)
	// and flatten all entries into a single PCollection.
	var encodings []beam.PCollection
	for _, table := range tables {
		encoded := beam.ParDo(s, &encodeKeyValue{beam.EncodedCoder{table.Coder()}}, table)
		encodings = append(encodings, encoded)
	}
	encoded := beam.Flatten(s, encodings...)

	// Group each key-value by a shard number based on its key's byte encoding.
	shards := beam.GroupByKey(s, beam.ParDo(s, &shardKeyValue{Shards: numShards}, encoded))

	// Write each shard to a separate SSTable.  The resulting PCollection contains
	// each SSTable's metadata (*tableMetadata).
	tableMetadata := beam.ParDo(s, &writeTable{path}, shards)

	// Write all SSTable metadata to the LevelDB's MANIFEST journal.
	beam.ParDo(s, &writeManifest{Path: path}, beam.GroupByKey(s, beam.AddFixedKey(s, tableMetadata)))
}

type writeManifest struct{ Path string }

type fsFile struct {
	io.WriteCloser
	fs filesystem.Interface
}

// Close implements part of the io.WriteCloser interface.  It closes both the
// file and underlying filesystem.
func (f *fsFile) Close() error {
	fErr := f.WriteCloser.Close()
	fsErr := f.fs.Close()
	if fErr != nil {
		return fErr
	}
	return fsErr
}

func openWrite(ctx context.Context, path string) (io.WriteCloser, error) {
	fs, err := filesystem.New(ctx, path)
	if err != nil {
		return nil, err
	}
	f, err := fs.OpenWrite(ctx, path)
	if err != nil {
		return nil, err
	}
	return &fsFile{f, fs}, nil
}

// Constants used as IDs for LevelDB journal entries.
const (
	manifestCompararerNum     = 1
	manifestCurrentJournalNum = 2
	manifestNextFileNum       = 3
	manifestLastCompactionNum = 4
	manifestAddedTableNum     = 7
)

// ProcessElement combines all tableMetadata into LevelDB's journal format and
// writes the database's CURRENT manifest file.
func (w *writeManifest) ProcessElement(ctx context.Context, _ beam.T, e func(*tableMetadata) bool) (int, error) {
	const manifestName = "MANIFEST-000000"
	defer func(start time.Time) { log.Printf("Manifest written in %s", time.Since(start)) }(time.Now())

	// Write the CURRENT manifest to the 0'th LevelDB file.
	f, err := openWrite(ctx, filepath.Join(w.Path, manifestName))
	if err != nil {
		return 0, err
	}

	journals := journal.NewWriter(f)
	j, err := journals.Next()
	if err != nil {
		return 0, err
	}

	// Comparer
	putUvarint(j, manifestCompararerNum)
	putBytes(j, []byte(keyComparer{}.Name()))

	// Current journal
	putUvarint(j, manifestCurrentJournalNum)
	putUvarint(j, 0) // MANIFEST-000000

	// Added table entry
	var maxShard, maxSeq int
	var md tableMetadata
	for e(&md) {
		putUvarint(j, manifestAddedTableNum)
		putUvarint(j, 0) // all SSTables are level-0
		putUvarint(j, uint64(md.Shard))
		putUvarint(j, uint64(md.Size))
		putBytes(j, md.First)
		putBytes(j, md.Last)

		// Keep track of the last shard num and maximum sequence number.
		if md.Shard > maxShard {
			maxShard = md.Shard
		}
		if md.Seq > maxSeq {
			maxSeq = md.Seq
		}
	}

	// Next available file entry
	putUvarint(j, manifestNextFileNum)
	putUvarint(j, uint64(maxShard+1))

	// Last compaction sequence
	putUvarint(j, manifestLastCompactionNum)
	putUvarint(j, uint64(maxSeq))

	if err := journals.Close(); err != nil {
		return 0, err
	} else if err := f.Close(); err != nil {
		return 0, err
	}

	// Write the CURRENT pointer to the freshly written manifest file.
	currentFile, err := openWrite(ctx, filepath.Join(w.Path, "CURRENT"))
	if err != nil {
		return 0, err
	} else if _, err := io.WriteString(currentFile, manifestName+"\n"); err != nil {
		return 0, err
	} else if err := currentFile.Close(); err != nil {
		return 0, err
	}

	return maxShard, nil
}

// putUvarint writes x as a varint to w.
func putUvarint(w io.Writer, x uint64) error {
	buf := make([]byte, binary.MaxVarintLen64)
	n := binary.PutUvarint(buf, x)
	_, err := w.Write(buf[:n])
	return err
}

// putBytes writes a varint-prefixed buffer to w.
func putBytes(w io.Writer, b []byte) error {
	if err := putUvarint(w, uint64(len(b))); err != nil {
		return err
	}
	_, err := w.Write(b)
	return err
}

type writeTable struct{ Path string }

// tableMetadata represents a single SSTable within a LevelDB.  Each SSTable
// written by the LevelDB sink is a level-0 table (meaning that its key ranges
// can overlap with another SSTable's).
type tableMetadata struct {
	Shard       int
	First, Last []byte
	Size        int
	Seq         int
}

var duplicateLevelDBKeysCounter = beam.NewCounter("kythe.beamio.leveldb", "duplicate-keys")

// ProcessElement writes a set of keyValues to the an SSTable per shard.  Shards
// should be small enough to fit into memory so that they can be sorted.
// TODO(BEAM-4405): use SortValues extension to remove in-memory requirement
func (w *writeTable) ProcessElement(ctx context.Context, shard int, e func(*keyValue) bool, emit func(tableMetadata)) error {
	opts := &opt.Options{
		BlockSize: 5 * opt.MiB,
		Comparer:  keyComparer{},
	}

	var totalElements int
	defer func(start time.Time) {
		log.Printf("Shard %04d: %s (size: %d)", shard, time.Since(start), totalElements)
	}(time.Now())
	md := tableMetadata{Shard: shard + 1}

	var els []keyValue
	var kv keyValue
	for e(&kv) {
		els = append(els, kv)
	}
	sort.Slice(els, func(i, j int) bool {
		return bytes.Compare(els[i].Key, els[j].Key) < 0
	})

	// Remove duplicate keys
	j := 1
	for i := 1; i < len(els); i++ {
		if bytes.Equal(els[j-1].Key, els[i].Key) {
			duplicateLevelDBKeysCounter.Inc(ctx, 1)
		} else {
			els[j] = els[i]
			j++
		}
	}
	els = els[:j]

	// Encode keys for LevelDB
	for i := 0; i < len(els); i++ {
		md.Seq++
		els[i].Key = makeLevelDBKey(uint64(md.Seq), els[i].Key)
	}

	totalElements = len(els)
	md.First = els[0].Key
	md.Last = els[len(els)-1].Key

	// Write each sorted key-value to an SSTable.
	f, err := openWrite(ctx, filepath.Join(w.Path, fmt.Sprintf("%06d.ldb", md.Shard)))
	if err != nil {
		return err
	}
	wr := table.NewWriter(f, opts)
	for _, kv := range els {
		if err := wr.Append(kv.Key, kv.Value); err != nil {
			return err
		}
	}
	if err := wr.Close(); err != nil {
		return err
	} else if err := f.Close(); err != nil {
		return err
	}
	md.Size = wr.BytesLen()

	emit(md)
	return nil
}

type shardKeyValue struct{ Shards int }

func (s *shardKeyValue) ProcessElement(kv keyValue) (int, keyValue) {
	h := crc32.NewIEEE()
	h.Write(kv.Key)
	return int(h.Sum32()) % s.Shards, kv
}

type encodeKeyValue struct {
	Coder beam.EncodedCoder
}

func (e *encodeKeyValue) ProcessElement(key beam.T, val beam.U) (keyValue, error) {
	c := beam.UnwrapCoder(e.Coder.Coder)
	keyEnc := exec.MakeElementEncoder(c.Components[0])
	var keyBuf bytes.Buffer
	if err := keyEnc.Encode(exec.FullValue{Elm: key}, &keyBuf); err != nil {
		return keyValue{}, err
	} else if _, err := binary.ReadUvarint(&keyBuf); err != nil {
		return keyValue{}, fmt.Errorf("error removing varint prefix from key encoding: %v", err)
	}
	valEnc := exec.MakeElementEncoder(c.Components[1])
	var valBuf bytes.Buffer
	if err := valEnc.Encode(exec.FullValue{Elm: val}, &valBuf); err != nil {
		return keyValue{}, err
	} else if _, err := binary.ReadUvarint(&valBuf); err != nil {
		return keyValue{}, fmt.Errorf("error removing varint prefix from value encoding: %v", err)
	}
	return keyValue{Key: keyBuf.Bytes(), Value: valBuf.Bytes()}, nil
}

// A keyValue is a concrete form of a Beam KV.
type keyValue struct {
	Key   []byte `json:"k"`
	Value []byte `json:"v"`
}

// makeLevelDBKey constructs an internal LevelDB key from a user key.  seq is
// the sequence number for the key-value entry within the LevelDB.
func makeLevelDBKey(seq uint64, key []byte) []byte {
	const typ = 1 // value (vs. deletion)
	k := make([]byte, len(key)+8)
	copy(k, key)
	binary.LittleEndian.PutUint64(k[len(key):], (seq<<8)|uint64(typ))
	return k
}

// parseLevelDBKey returns the user key and the sequence number (and value type)
// from an internal LevelDB key.
func parseLevelDBKey(key []byte) (ukey []byte, seqNum uint64) {
	return key[:len(key)-8], binary.LittleEndian.Uint64(key[len(key)-8:])
}

// keyComparer compares internal (ukey, seqNum) LevelDB keys.
type keyComparer struct{}

// Name implements part of the comparer.Comparer interface.
func (keyComparer) Name() string { return "leveldb.BytewiseComparator" }

// Compare implements part of the comparer.Comparer interface.
func (keyComparer) Compare(a, b []byte) int {
	ak, an := parseLevelDBKey(a)
	bk, bn := parseLevelDBKey(b)
	c := bytes.Compare(ak, bk)
	if c == 0 {
		return int(bn - an)
	}
	return c
}

// Separator implements part of the comparer.Comparer interface.
func (keyComparer) Separator(dst, a, b []byte) []byte {
	ak, _ := parseLevelDBKey(a)
	bk, _ := parseLevelDBKey(b)
	dst = comparer.DefaultComparer.Separator(dst, ak, bk)
	if dst != nil && len(dst) < len(ak) && bytes.Compare(ak, dst) < 0 {
		return append(dst, maxKeyNumSuffix...)
	}
	return nil
}

// Successor implements part of the comparer.Comparer interface.
func (keyComparer) Successor(dst, k []byte) []byte {
	k, _ = parseLevelDBKey(k)
	dst = comparer.DefaultComparer.Successor(dst, k)
	if dst != nil && len(dst) < len(k) && bytes.Compare(k, dst) < 0 {
		return append(dst, maxKeyNumSuffix...)
	}
	return nil
}

// maxKeyNumSuffix is maximum possible sequence number (and value type) for an
// internal LevelDB key.
var maxKeyNumSuffix = bytes.Repeat([]byte{0xFF}, 8)
