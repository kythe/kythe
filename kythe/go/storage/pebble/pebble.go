// Package pebble implements a graphstore.Service using a Pebble backend
// database.
package pebble // import "kythe.io/kythe/go/storage/pebble"

import (
	"context"
	"fmt"
	"io"
	
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/storage/gsutil"
	"kythe.io/kythe/go/storage/keyvalue"

	"github.com/cockroachdb/pebble"
	"github.com/cockroachdb/pebble/vfs"
)

func init() {
	gsutil.Register("pebble", func(spec string) (graphstore.Service, error) { return OpenGraphStore(spec, nil) })
	gsutil.RegisterDefault("pebble")
}

type pebbleDB struct {
	db *pebble.DB
	dbOpts *Options
}

// CompactRange runs a manual compaction on the Range of keys given.
// If r == nil, the entire table will be compacted.
func CompactRange(path string, r *keyvalue.Range) error {
	db, err := pebble.Open(path, &pebble.Options{})
	if err != nil {
		return err
	}
	defer db.Close()

	start := []byte{byte(0)}
	end := []byte{byte(255)}
	if r != nil {
		start = r.Start
		end = r.End
	}
	return db.Compact(start, end, true/*=parallelize*/)
}

var DefaultOptions = &Options{
	CacheCapacity:   512 * 1024 * 1024, // 512mb
	WriteBufferSize: 60 * 1024 * 1024,  // 60mb
}

// Options for customizing a LevelDB backend.
type Options struct {
	// CacheCapacity is the caching capacity (in bytes) used for the LevelDB.
	CacheCapacity int64

	// CacheLargeReads determines whether to use the cache for large reads. This
	// is usually discouraged but may be useful when the entire LevelDB is known
	// to fit into the cache.
	CacheLargeReads bool

	// WriteBufferSize is the number of bytes the database will build up in memory
	// (backed by a disk log) before writing to the on-disk table.
	WriteBufferSize int

	// MustExist ensures that the given database exists before opening it.  If
	// false and the database does not exist, it will be created.
	MustExist bool
}

// ValidDB determines if the given path could be a LevelDB database.
func ValidDB(path string) bool {
	desc, err := pebble.Peek(path, vfs.Default)
        if err != nil {
		return false
        }
	return desc.Exists
}

// OpenGraphStore returns a graphstore.Service backed by a LevelDB database at
// the given filepath.  If opts==nil, the DefaultOptions are used.
func OpenGraphStore(path string, opts *Options) (graphstore.Service, error) {
	if opts.MustExist && !ValidDB(path) {
		return nil, fmt.Errorf("database %q did not exist", path)
	}
	db, err := Open(path, opts)
	if err != nil {
		return nil, err
	}
	return keyvalue.NewGraphStore(db), nil
}

// Open returns a keyvalue DB backed by a Pebble database at the given
// filepath.  If opts==nil, the DefaultOptions are used.
func Open(path string, opts *Options) (keyvalue.DB, error) {
	if opts == nil {
		opts = DefaultOptions
	}
	c := pebble.NewCache(opts.CacheCapacity)
	defer c.Unref()

	db, err := pebble.Open(path, &pebble.Options{Cache: c})
	if err != nil {
		return nil, err
	}
	
	return &pebbleDB{
		db: db,
	}, nil
}

// Close will close the underlying pebble database.
func (p *pebbleDB) Close(_ context.Context) error {
	return p.db.Close()
}

func (p *pebbleDB) readerForOpts(opts *keyvalue.Options) pebble.Reader {
	if snap := opts.GetSnapshot(); snap != nil {
		return snap.(*pebble.Snapshot)
	}
	return p.db
}

func (p *pebbleDB) Get(ctx context.Context, key []byte, opts *keyvalue.Options) ([]byte, error) {
	r := p.readerForOpts(opts)
	value, closer, err := r.Get(key)
	if err != nil {
		if err == pebble.ErrNotFound {
			return nil, io.EOF
		}
		return nil, err
	}
	defer closer.Close()
	return value, nil
}

func (p *pebbleDB) NewSnapshot(ctx context.Context) keyvalue.Snapshot {
	snap := p.db.NewSnapshot()
	return snap
}

type pebbleWriter struct {
	*pebble.Batch
}
func (pw *pebbleWriter) Write(key, val []byte) error {
	pw.Batch.Set(key, val, nil/*=ignored write options*/)
	return nil
}
func (pw *pebbleWriter) Close() error {
	defer pw.Batch.Close()
	if err := pw.Batch.Commit(pebble.NoSync); err != nil {
		return err
	}
	return nil
}

func (p *pebbleDB) Writer(_ context.Context) (keyvalue.Writer, error) {
	return &pebbleWriter{p.db.NewBatch()}, nil
}

type pebbleIter struct {
	*pebble.Iterator
}
func (pi *pebbleIter) Next() ([]byte, []byte, error) {
	if pi.Iterator.Valid() {
		key := make([]byte, len(pi.Iterator.Key()))
		val := make([]byte, len(pi.Iterator.Value()))
		copy(key, pi.Iterator.Key())
		copy(val, pi.Iterator.Value())		
		pi.Iterator.Next()
		return key, val, nil
	}
	return nil, nil, io.EOF	
}
func (pi *pebbleIter) Seek(key []byte) error {
	if !pi.Iterator.SeekGE(key) {
		return io.EOF
	}
	return nil
}

func (p *pebbleDB) ScanPrefix(_ context.Context, prefix []byte, opts *keyvalue.Options) (keyvalue.Iterator, error) {
	r := p.readerForOpts(opts)
	iter, err := r.NewIter(nil)
	if err != nil {
		return nil, err
	}
	if len(prefix) == 0 {
		iter.First()
	} else {
		iter.SeekGE(prefix)
	}
	return &pebbleIter{iter}, nil
}

func (p *pebbleDB) ScanRange(_ context.Context, rng *keyvalue.Range, opts *keyvalue.Options) (keyvalue.Iterator, error) {
	r := p.readerForOpts(opts)
	iter, err := r.NewIter(&pebble.IterOptions{
		LowerBound: rng.Start,
		UpperBound: rng.End,
	})
	if err != nil {
		return nil, err
	}
	iter.First()
	return &pebbleIter{iter}, nil
}
