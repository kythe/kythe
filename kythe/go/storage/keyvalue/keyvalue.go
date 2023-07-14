/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

// Package keyvalue implements a generic GraphStore for anything that implements
// the DB interface.
package keyvalue // import "kythe.io/kythe/go/storage/keyvalue"

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"io"
	"strings"
	"sync"

	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/util/datasize"
	"kythe.io/kythe/go/util/log"

	spb "kythe.io/kythe/proto/storage_go_proto"
)

// debug controls whether debugging information is emitted
const debug = false

// A Store implements the graphstore.Service interface for a keyvalue DB
type Store struct {
	db DB

	shardMu        sync.Mutex // guards shardTables/shardSnapshots during construction
	shardTables    map[int64][]shard
	shardSnapshots map[int64]Snapshot
}

// Range is section of contiguous keys, including Start and excluding End.
type Range struct {
	Start, End []byte
}

// KeyRange returns a Range that contains only the given key.
func KeyRange(k []byte) *Range {
	return &Range{
		Start: k,
		End:   append(k[0:len(k):len(k)], 0),
	}
}

type shard struct {
	Range
	count int64
}

// NewGraphStore returns a graphstore.Service backed by the given keyvalue DB.
func NewGraphStore(db DB) *Store {
	return &Store{db: db}
}

// A DB is a sorted key-value store with read/write access. DBs must be Closed
// when no longer used to ensure resources are not leaked.
type DB interface {
	// Get returns the value associated with the given key.  An io.EOF will be
	// returned if the key is not found.
	Get(context.Context, []byte, *Options) ([]byte, error)

	// ScanPrefix returns an Iterator for all key-values starting with the given
	// key prefix.  Options may be nil to use the defaults.
	ScanPrefix(context.Context, []byte, *Options) (Iterator, error)

	// ScanRange returns an Iterator for all key-values starting with the given
	// key range.  Options may be nil to use the defaults.
	ScanRange(context.Context, *Range, *Options) (Iterator, error)

	// Writer return a new write-access object
	Writer(context.Context) (Writer, error)

	// NewSnapshot returns a new consistent view of the DB that can be passed as
	// an option to DB scan methods.
	NewSnapshot(context.Context) Snapshot

	// Close release the underlying resources for the database.
	Close(context.Context) error
}

// Snapshot is a consistent view of the DB.
type Snapshot io.Closer

// Options alters the behavior of an Iterator.
type Options struct {
	// LargeRead expresses the client's intent that the read will likely be
	// "large" and the implementation should usually avoid certain behaviors such
	// as caching the entire visited key-value range.  Defaults to false.
	LargeRead bool

	// Snapshot causes the iterator to view the DB as it was at the Snapshot's
	// creation.
	Snapshot
}

// IsLargeRead returns the LargeRead option or the default of false when o==nil.
func (o *Options) IsLargeRead() bool {
	return o != nil && o.LargeRead
}

// GetSnapshot returns the Snapshot option or the default of nil when o==nil.
func (o *Options) GetSnapshot() Snapshot {
	if o == nil {
		return nil
	}
	return o.Snapshot
}

// Iterator provides sequential access to a DB. Iterators must be Closed when
// no longer used to ensure that resources are not leaked.
type Iterator interface {
	io.Closer

	// Next returns the currently positioned key-value entry and moves to the next
	// entry. If there is no key-value entry to return, an io.EOF error is
	// returned.
	Next() (key, val []byte, err error)

	// Seeks positions the Iterator to the given key.  The key must be further
	// than the current Iterator's position.  If the key does not exist, the
	// Iterator is positioned at the next existing key.  If no such key exists,
	// io.EOF is returned.
	Seek(key []byte) error
}

// Writer provides write access to a DB. Writes must be Closed when no longer
// used to ensure that resources are not leaked.
type Writer interface {
	io.Closer

	// Write writes a key-value entry to the DB. Writes may be batched until the
	// Writer is Closed.
	Write(key, val []byte) error
}

// WritePool is a wrapper around a DB that automatically creates and flushes
// Writers as data size is written, creating a simple buffered interface for
// writing to a DB.  This interface is not thread-safe.
type WritePool struct {
	db   DB
	opts *PoolOptions

	wr     Writer
	writes int
	size   uint64
}

// PoolOptions is a set of options used by WritePools.
type PoolOptions struct {
	// MaxWrites is the number of calls to Write before the WritePool
	// automatically flushes the underlying Writer.  This defaults to 32000
	// writes.
	MaxWrites int

	// MaxSize is the total size of the keys and values given to Write before the
	// WritePool automatically flushes the underlying Writer.  This defaults to
	// 32MiB.
	MaxSize datasize.Size
}

func (o *PoolOptions) maxWrites() int {
	if o == nil || o.MaxWrites <= 0 {
		return 32000
	}
	return o.MaxWrites
}

func (o *PoolOptions) maxSize() uint64 {
	if o == nil || o.MaxSize <= 0 {
		return (datasize.Mebibyte * 32).Bytes()
	}
	return o.MaxSize.Bytes()
}

// NewPool returns a new WritePool for the given DB.  If opts==nil, its defaults
// are used.
func NewPool(db DB, opts *PoolOptions) *WritePool { return &WritePool{db: db, opts: opts} }

// Write buffers the given write until the pool becomes to large or Flush is
// called.
func (p *WritePool) Write(ctx context.Context, key, val []byte) error {
	if p.wr == nil {
		wr, err := p.db.Writer(ctx)
		if err != nil {
			return err
		}
		p.wr = wr
	}
	if err := p.wr.Write(key, val); err != nil {
		return err
	}
	p.size += uint64(len(key)) + uint64(len(val))
	p.writes++
	if p.opts.maxWrites() <= p.writes || p.opts.maxSize() <= p.size {
		return p.Flush()
	}
	return nil
}

// Flush ensures that all buffered writes are applied to the underlying DB.
func (p *WritePool) Flush() error {
	if p.wr == nil {
		return nil
	}
	if debug {
		log.Infof("Flushing (%d) %s", p.writes, datasize.Size(p.size))
	}
	err := p.wr.Close()
	p.wr = nil
	p.size, p.writes = 0, 0
	return err
}

// Read implements part of the graphstore.Service interface.
func (s *Store) Read(ctx context.Context, req *spb.ReadRequest, f graphstore.EntryFunc) error {
	keyPrefix, err := KeyPrefix(req.Source, req.EdgeKind)
	if err != nil {
		return fmt.Errorf("invalid ReadRequest: %v", err)
	}
	iter, err := s.db.ScanPrefix(ctx, keyPrefix, nil)
	if err != nil {
		return fmt.Errorf("db seek error: %v", err)
	}
	return streamEntries(iter, f)
}

func streamEntries(iter Iterator, f graphstore.EntryFunc) error {
	defer iter.Close()
	for {
		key, val, err := iter.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("db iteration error: %v", err)
		}

		entry, err := Entry(key, val)
		if err != nil {
			return fmt.Errorf("encoding error: %v", err)
		}
		if err := f(entry); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
	return nil
}

// Write implements part of the GraphStore interface.
func (s *Store) Write(ctx context.Context, req *spb.WriteRequest) (err error) {
	// TODO(schroederc): fix shardTables to include new entries

	wr, err := s.db.Writer(ctx)
	if err != nil {
		return fmt.Errorf("db writer error: %v", err)
	}
	defer func() {
		cErr := wr.Close()
		if err == nil && cErr != nil {
			err = fmt.Errorf("db writer close error: %v", cErr)
		}
	}()
	for _, update := range req.Update {
		if update.FactName == "" {
			return errors.New("invalid WriteRequest: Update missing FactName")
		}
		updateKey, err := EncodeKey(req.Source, update.FactName, update.EdgeKind, update.Target)
		if err != nil {
			return fmt.Errorf("encoding error: %v", err)
		}
		if err := wr.Write(updateKey, update.FactValue); err != nil {
			return fmt.Errorf("db write error: %v", err)
		}
	}
	return nil
}

// Scan implements part of the graphstore.Service interface.
func (s *Store) Scan(ctx context.Context, req *spb.ScanRequest, f graphstore.EntryFunc) error {
	iter, err := s.db.ScanPrefix(ctx, entryKeyPrefixBytes, &Options{LargeRead: true})
	if err != nil {
		return fmt.Errorf("db seek error: %v", err)
	}
	defer iter.Close()
	for {
		key, val, err := iter.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("db iteration error: %v", err)
		}
		entry, err := Entry(key, val)
		if err != nil {
			return fmt.Errorf("invalid key/value entry: %v", err)
		}
		if !graphstore.EntryMatchesScan(req, entry) {
			continue
		} else if err := f(entry); err == io.EOF {
			return nil
		} else if err != nil {
			return err
		}
	}
	return nil
}

// Close implements part of the graphstore.Service interface.
func (s *Store) Close(ctx context.Context) error { return s.db.Close(ctx) }

// Count implements part of the graphstore.Sharded interface.
func (s *Store) Count(ctx context.Context, req *spb.CountRequest) (int64, error) {
	if req.Shards < 1 {
		return 0, fmt.Errorf("invalid number of shards: %d", req.Shards)
	} else if req.Index < 0 || req.Index >= req.Shards {
		return 0, fmt.Errorf("invalid index for %d shards: %d", req.Shards, req.Index)
	}

	tbl, _, err := s.constructShards(ctx, req.Shards)
	if err != nil {
		return 0, err
	}
	return tbl[req.Index].count, nil
}

// Shard implements part of the graphstore.Sharded interface.
func (s *Store) Shard(ctx context.Context, req *spb.ShardRequest, f graphstore.EntryFunc) error {
	if req.Shards < 1 {
		return fmt.Errorf("invalid number of shards: %d", req.Shards)
	} else if req.Index < 0 || req.Index >= req.Shards {
		return fmt.Errorf("invalid index for %d shards: %d", req.Shards, req.Index)
	}

	tbl, snapshot, err := s.constructShards(ctx, req.Shards)
	if err != nil {
		return err
	}
	if tbl[req.Index].count == 0 {
		return nil
	}
	shard := tbl[req.Index]
	iter, err := s.db.ScanRange(ctx, &shard.Range, &Options{
		LargeRead: true,
		Snapshot:  snapshot,
	})
	if err != nil {
		return err
	}
	return streamEntries(iter, f)
}

func (s *Store) constructShards(ctx context.Context, num int64) ([]shard, Snapshot, error) {
	s.shardMu.Lock()
	defer s.shardMu.Unlock()
	if s.shardTables == nil {
		s.shardTables = make(map[int64][]shard)
		s.shardSnapshots = make(map[int64]Snapshot)
	}
	if tbl, ok := s.shardTables[num]; ok {
		return tbl, s.shardSnapshots[num], nil
	}
	snapshot := s.db.NewSnapshot(ctx)
	iters := make([]Iterator, num)
	for i := range iters {
		var err error
		iters[i], err = s.db.ScanPrefix(ctx, entryKeyPrefixBytes, &Options{
			LargeRead: true,
			Snapshot:  snapshot,
		})
		if err != nil {
			snapshot.Close()
			return nil, nil, fmt.Errorf("error creating iterator: %v", err)
		}
	}

	// This loop determines the ending key to each shard's range and the number of
	// entries each iterator has passed.  Each iterator always represents the
	// current ending key to each shard and is moved in (i+1) groups of entries
	// where i is its index in iters/tbl.  If a group consisted of a single entry,
	// this staggered iteration evenly distribute the iterators across the entire
	// GraphStore.  However, this loop iterates past groups of entries sharing the
	// same (source+edgeKind) at a time to ensure the property that no node/edge
	// crosses a shard boundary.  This also means that the shards will be less
	// evenly distributed.
	tbl := make([]shard, num)
loop:
	for { // Until an iterator (usually iters[num-1]) reaches io.EOF
		for i, iter := range iters {
			// Move iters[i] past i+1 sets of entries sharing the same
			// (source+edgeKind) prefix.
			for j := 0; j <= i; j++ {
				k, _, err := iter.Next()
				if err == io.EOF {
					break loop
				} else if err != nil {
					snapshot.Close()
					return nil, nil, err
				}
				prefix := sourceKindPrefix(k)
				tbl[i].count++
				tbl[i].End = k

				// Iterate past all entries with the same source+kind prefix as k
				for {
					k, _, err = iter.Next()
					if err == io.EOF {
						break loop
					} else if err != nil {
						snapshot.Close()
						return nil, nil, err
					}
					tbl[i].count++
					tbl[i].End = k
					if !bytes.HasPrefix(k, prefix) {
						break
					}
				}
			}
		}
	}

	// Fix up the border shards
	tbl[0].Start = entryKeyPrefixBytes
	tbl[num-1].End = entryKeyPrefixEndRange
	tbl[0].count--
	tbl[num-1].count++

	// Set the starting keys to each shard.
	for i := int64(1); i < num; i++ {
		tbl[i].Start = tbl[i-1].End
	}
	// Determine the size of each shard.
	for i := num - 1; i > 0; i-- {
		tbl[i].count -= tbl[i-1].count
	}

	s.shardTables[num] = tbl
	s.shardSnapshots[num] = snapshot
	return tbl, snapshot, nil
}

func sourceKindPrefix(key []byte) []byte {
	idx := bytes.IndexRune(key, entryKeySep)
	return key[:bytes.IndexRune(key[idx+1:], entryKeySep)+idx+2]
}

// GraphStore Implementation Details:
//   These details are strictly for this particular implementation of a
//   GraphStore and are *not* specified in the GraphStore specification.  These
//   particular encodings, however, do satisfy the GraphStore requirements,
//   including the GraphStore entry ordering property.  Also, due to the
//   "entry:" key prefix, this implementation allows for additional embedded
//   GraphStore metadata/indices using other distinct key prefixes.
//
//   The encoding format for entries in a keyvalue GraphStore is:
//     "entry:<source>_<edgeKind>_<factName>_<target>" == "<factValue>"
//   where:
//     "entry:" == entryKeyPrefix
//     "_"      == entryKeySep
//     <source> and <target> are the Entry's encoded VNames:
//
//   The encoding format for VNames is:
//     <signature>-<corpus>-<root>-<path>-<language>
//   where:
//     "-"      == vNameFieldSep

const (
	entryKeyPrefix = "entry:"

	// entryKeySep is used to separate the source, factName, edgeKind, and target of an
	// encoded Entry key
	entryKeySep    = '\n'
	entryKeySepStr = string(entryKeySep)

	// vNameFieldSep is used to separate the fields of an encoded VName
	vNameFieldSep = "\000"
)

var (
	entryKeyPrefixBytes    = []byte(entryKeyPrefix)
	entryKeyPrefixEndRange = append([]byte(entryKeyPrefix[:len(entryKeyPrefixBytes)-1]), entryKeyPrefix[len(entryKeyPrefixBytes)-1]+1)
	entryKeySepBytes       = []byte{entryKeySep}
)

// EncodeKey returns a canonical encoding of an Entry (minus its value).
func EncodeKey(source *spb.VName, factName string, edgeKind string, target *spb.VName) ([]byte, error) {
	if source == nil {
		return nil, errors.New("invalid Entry: missing source VName for key encoding")
	} else if (edgeKind == "" || target == nil) && (edgeKind != "" || target != nil) {
		return nil, errors.New("invalid Entry: edgeKind and target Ticket must be both non-empty or empty")
	} else if strings.Contains(edgeKind, entryKeySepStr) {
		return nil, errors.New("invalid Entry: edgeKind contains key separator")
	} else if strings.Contains(factName, entryKeySepStr) {
		return nil, errors.New("invalid Entry: factName contains key separator")
	}

	keySuffix := []byte(entryKeySepStr + edgeKind + entryKeySepStr + factName + entryKeySepStr)

	srcEncoding, err := encodeVName(source)
	if err != nil {
		return nil, fmt.Errorf("error encoding source VName: %v", err)
	} else if bytes.Contains(srcEncoding, entryKeySepBytes) {
		return nil, fmt.Errorf("invalid Entry: source VName contains key separator (%q) %v", entryKeySepBytes, source)
	}
	targetEncoding, err := encodeVName(target)
	if err != nil {
		return nil, fmt.Errorf("error encoding target VName: %v", err)
	} else if bytes.Contains(targetEncoding, entryKeySepBytes) {
		return nil, errors.New("invalid Entry: target VName contains key separator")
	}

	return bytes.Join([][]byte{
		entryKeyPrefixBytes,
		srcEncoding,
		keySuffix,
		targetEncoding,
	}, nil), nil
}

// KeyPrefix returns a prefix to every encoded key for the given source VName and exact
// edgeKind. If edgeKind is "*", the prefix will match any edgeKind.
func KeyPrefix(source *spb.VName, edgeKind string) ([]byte, error) {
	if source == nil {
		return nil, errors.New("missing source VName")
	}
	srcEncoding, err := encodeVName(source)
	if err != nil {
		return nil, fmt.Errorf("error encoding source VName: %v", err)
	}

	prefix := bytes.Join([][]byte{entryKeyPrefixBytes, append(srcEncoding, entryKeySep)}, nil)
	if edgeKind == "*" {
		return prefix, nil
	}

	return bytes.Join([][]byte{prefix, append([]byte(edgeKind), entryKeySep)}, nil), nil
}

// Entry decodes the key (assuming it was encoded by EncodeKey) into an Entry
// and populates its value field.
func Entry(key []byte, val []byte) (*spb.Entry, error) {
	if !bytes.HasPrefix(key, entryKeyPrefixBytes) {
		return nil, fmt.Errorf("key is not prefixed with entry prefix %q", entryKeyPrefix)
	}
	keyStr := string(bytes.TrimPrefix(key, entryKeyPrefixBytes))
	keyParts := strings.SplitN(keyStr, entryKeySepStr, 4)
	if len(keyParts) != 4 {
		return nil, fmt.Errorf("invalid key[%d]: %q", len(keyParts), string(key))
	}

	srcVName, err := decodeVName(keyParts[0])
	if err != nil {
		return nil, fmt.Errorf("error decoding source VName: %v", err)
	}
	targetVName, err := decodeVName(keyParts[3])
	if err != nil {
		return nil, fmt.Errorf("error decoding target VName: %v", err)
	}

	return &spb.Entry{
		Source:    srcVName,
		FactName:  keyParts[2],
		EdgeKind:  keyParts[1],
		Target:    targetVName,
		FactValue: val,
	}, nil
}

// encodeVName returns a canonical byte array for the given VName. Returns nil if given nil.
func encodeVName(v *spb.VName) ([]byte, error) {
	if v == nil {
		return nil, nil
	} else if strings.Contains(v.Signature, vNameFieldSep) ||
		strings.Contains(v.Corpus, vNameFieldSep) ||
		strings.Contains(v.Root, vNameFieldSep) ||
		strings.Contains(v.Path, vNameFieldSep) ||
		strings.Contains(v.Language, vNameFieldSep) {
		return nil, fmt.Errorf("VName contains invalid rune: %q", vNameFieldSep)
	}
	return []byte(strings.Join([]string{
		v.Signature,
		v.Corpus,
		v.Root,
		v.Path,
		v.Language,
	}, vNameFieldSep)), nil
}

// decodeVName returns the VName coded in the given string. Returns nil, if len(data) == 0.
func decodeVName(data string) (*spb.VName, error) {
	if len(data) == 0 {
		return nil, nil
	}
	parts := strings.SplitN(data, vNameFieldSep, 5)
	if len(parts) != 5 {
		return nil, fmt.Errorf("invalid VName encoding: %q", data)
	}
	return &spb.VName{
		Signature: parts[0],
		Corpus:    parts[1],
		Root:      parts[2],
		Path:      parts[3],
		Language:  parts[4],
	}, nil
}
