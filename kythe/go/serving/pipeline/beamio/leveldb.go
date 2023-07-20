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

package beamio

import (
	"bytes"
	"context"
	"encoding/binary"
	"fmt"
	"io"
	"path/filepath"
	"reflect"
	"sort"
	"strings"
	"time"

	"kythe.io/kythe/go/util/log"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/io/filesystem"
	"github.com/apache/beam/sdks/go/pkg/beam/transforms/stats"
	"github.com/syndtr/goleveldb/leveldb/comparer"
	"github.com/syndtr/goleveldb/leveldb/journal"
	"github.com/syndtr/goleveldb/leveldb/opt"
	"github.com/syndtr/goleveldb/leveldb/table"
)

func init() {
	beam.RegisterType(reflect.TypeOf((*writeManifest)(nil)).Elem())
	beam.RegisterType(reflect.TypeOf((*writeTable)(nil)).Elem())
	beam.RegisterFunction(keyByKey)
	beam.RegisterFunction(distinctCombine)
}

// WriteLevelDB writes a set of PCollections containing KVs to a new LevelDB at
// the given path.  Each KV is serialized and stored as a single LevelDB
// key-value entry according to their enclosing PCollection's beam.Coder.  Each
// table may have different KV types.  Keys must be unique across all
// PCollections.
func WriteLevelDB(s beam.Scope, path string, opts stats.Opts, tables ...beam.PCollection) {
	filesystem.ValidateScheme(path)
	s = s.Scope("WriteLevelDB")

	tableMetadata := writeShards(s, path, opts, tables...)

	// Write all SSTable metadata to the LevelDB's MANIFEST journal.
	s = s.Scope("Manifest")
	beam.ParDo(s, &writeManifest{Path: path}, beam.GroupByKey(s, beam.AddFixedKey(s, tableMetadata)))
}

func writeShards(s beam.Scope, path string, opts stats.Opts, tables ...beam.PCollection) beam.PCollection {
	s = s.Scope("Shards")

	encoded := EncodeKeyValues(s, tables...)

	// Group each key-value by a shard number based on its key's byte encoding.
	shards := beam.GroupByKey(s, ComputeShards(s, makeDistinct(s, encoded), opts))

	// Write each shard to a separate SSTable.  The resulting PCollection contains
	// each SSTable's metadata (*tableMetadata).
	return beam.ParDo(s, &writeTable{path}, shards)
}

func keyByKey(kv KeyValue) ([]byte, KeyValue) {
	return kv.Key, kv
}

func makeDistinct(s beam.Scope, kvs beam.PCollection) beam.PCollection {
	return beam.DropKey(s, beam.CombinePerKey(s, distinctCombine, beam.ParDo(s, keyByKey, kvs)))
}

func distinctCombine(ctx context.Context, accum, other KeyValue) KeyValue {
	if accum.Key == nil {
		return other
	}
	duplicateLevelDBKeysCounter.Inc(ctx, 1)
	if !bytes.Equal(accum.Value, other.Value) {
		conflictingLevelDBValuesCounter.Inc(ctx, 1)
	}
	return accum
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
// writes the database's CURRENT manifest file.  It returns the maximum shard
// number processed.
func (w *writeManifest) ProcessElement(ctx context.Context, _ beam.T, e func(*tableMetadata) bool) (int, error) {
	const manifestName = "MANIFEST-000000"
	defer func(start time.Time) { log.Infof("Manifest written in %s", time.Since(start)) }(time.Now())

	// Write the CURRENT manifest to the 0'th LevelDB file.
	f, err := openWrite(ctx, schemePreservingPathJoin(w.Path, manifestName))
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
	currentFile, err := openWrite(ctx, schemePreservingPathJoin(w.Path, "CURRENT"))
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
	// Shard is the table's identifying number.
	Shard int

	// First/Last are the first and last keys in the table.
	First, Last []byte

	// Size is the byte size of the encoded table.
	Size int

	// Seq is the last used sequence number in the table.
	Seq int
}

var (
	duplicateLevelDBKeysCounter     = beam.NewCounter("kythe.beamio.leveldb", "duplicate-keys")
	conflictingLevelDBValuesCounter = beam.NewCounter("kythe.beamio.leveldb", "conflicting-values")
)

const schemaSeparator = "://"

// schemePreservingPathJoin is like filepath.Join, but doesn't collapse
// the double-slash in the schema prefix, if any.
func schemePreservingPathJoin(p, f string) string {
	parts := strings.SplitN(p, schemaSeparator, 2)
	if len(parts) == 2 {
		return parts[0] + schemaSeparator + filepath.Join(parts[1], f)
	}
	return filepath.Join(p, f)
}

// ProcessElement writes a set of KeyValues to the an SSTable per shard.  Shards
// should be small enough to fit into memory so that they can be sorted.
// TODO(BEAM-4405): use SortValues extension to remove in-memory requirement
func (w *writeTable) ProcessElement(ctx context.Context, shard int, kvIter func(*KeyValue) bool, emit func(tableMetadata)) error {
	opts := &opt.Options{
		BlockSize: 5 * opt.MiB,
		Comparer:  keyComparer{},
	}

	var totalElements int
	defer func(start time.Time) {
		log.Infof("Shard %04d: %s (size: %d)", shard, time.Since(start), totalElements)
	}(time.Now())
	md := tableMetadata{Shard: shard + 1}

	var els []KeyValue
	var kv KeyValue
	for kvIter(&kv) {
		els = append(els, kv)
	}
	sort.Slice(els, func(i, j int) bool {
		return bytes.Compare(els[i].Key, els[j].Key) < 0
	})

	// Remove duplicate keys
	j := 1
	for i := 1; i < len(els); i++ {
		if bytes.Equal(els[j-1].Key, els[i].Key) {
			if !bytes.Equal(els[j-1].Value, els[i].Value) {
				conflictingLevelDBValuesCounter.Inc(ctx, 1)
			}
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
	f, err := openWrite(ctx, schemePreservingPathJoin(w.Path, fmt.Sprintf("%06d.ldb", md.Shard)))
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

const keySuffixSize = 8

// makeLevelDBKey constructs an internal LevelDB key from a user key.  seq is
// the sequence number for the key-value entry within the LevelDB.
func makeLevelDBKey(seq uint64, key []byte) []byte {
	const typ = 1 // value (vs. deletion)
	k := make([]byte, len(key)+keySuffixSize)
	copy(k, key)
	binary.LittleEndian.PutUint64(k[len(key):], (seq<<keySuffixSize)|typ)
	return k
}

// parseLevelDBKey returns the user key and the sequence number (and value type)
// from an internal LevelDB key.
func parseLevelDBKey(key []byte) (ukey []byte, seqNum uint64) {
	return key[:len(key)-keySuffixSize], binary.LittleEndian.Uint64(key[len(key)-keySuffixSize:])
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
var maxKeyNumSuffix = bytes.Repeat([]byte{0xFF}, keySuffixSize)
