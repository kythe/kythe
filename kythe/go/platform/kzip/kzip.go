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

// Package kzip implements the kzip compilation storage file format.
//
// The package exports two types of interest: A kzip.Reader can be used to read
// the contents of an existing kzip archive, and a kzip.Writer can be used to
// construct a new kzip archive.
//
// Reading an Archive:
//
//	r, err := kzip.NewReader(file, size)
//	...
//
//	// Look up a compilation record by its digest.
//	unit, err := r.Lookup(unitDigest)
//	...
//
//	// Scan all the compilation records stored.
//	err := r.Scan(func(unit *kzip.Unit) error {
//	   if hasInterestingProperty(unit) {
//	      doStuffWith(unit)
//	   }
//	   return nil
//	})
//
//	// Open a reader for a stored file.
//	rc, err := r.Open(fileDigest)
//	...
//	defer rc.Close()
//
//	// Read the complete contents of a stored file.
//	bits, err := r.ReadAll(fileDigest)
//	...
//
// Writing an Archive:
//
//	w, err := kzip.NewWriter(file)
//	...
//
//	// Add a compilation record and (optional) index data.
//	udigest, err := w.AddUnit(unit, nil)
//	...
//
//	// Add file contents.
//	fdigest, err := w.AddFile(file)
//	...
package kzip // import "kythe.io/kythe/go/platform/kzip"

import (
	"archive/zip"
	"bytes"
	"context"
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	"kythe.io/kythe/go/platform/kcd/kythe"
	"kythe.io/kythe/go/util/log"
	"kythe.io/kythe/go/util/ptypes"

	"bitbucket.org/creachadair/stringset"
	"github.com/golang/protobuf/proto"
	"golang.org/x/sync/errgroup"
	"google.golang.org/protobuf/encoding/protojson"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"

	// These are common detail messages used by Kythe compilations, and
	// required for JSON (un)marshaling to work.
	_ "kythe.io/kythe/proto/buildinfo_go_proto"
	_ "kythe.io/kythe/proto/cxx_go_proto"
	_ "kythe.io/kythe/proto/filecontext_go_proto"
	_ "kythe.io/kythe/proto/go_go_proto"
	_ "kythe.io/kythe/proto/java_go_proto"
)

// Encoding describes how compilation units will be encoded when written to a kzip.
type Encoding int

const (
	// EncodingJSON specifies to use JSON encoding
	EncodingJSON Encoding = 1
	// EncodingProto specifies to use Proto encoding
	EncodingProto Encoding = 2
	// EncodingAll specifies to encode using all known encodings
	EncodingAll Encoding = EncodingJSON | EncodingProto

	prefixJSON  = "units"
	prefixProto = "pbunits"
)

// Compilation is a CompilationUnit with the contents for all of its required inputs.
type Compilation struct {
	Proto *apb.CompilationUnit `json:"compilation"`
	Files []*apb.FileData      `json:"files"`
}

var (
	// Use a constant file modification time in the kzip so file diffs only compare the contents,
	// not when the kzips were created.
	modifiedTime = time.Unix(0, 0)
)

// EncodingFor converts a string to an Encoding.
func EncodingFor(v string) (Encoding, error) {
	v = strings.ToUpper(v)
	switch {
	case v == "ALL":
		return EncodingAll, nil
	case v == "JSON":
		return EncodingJSON, nil
	case v == "PROTO":
		return EncodingProto, nil
	default:
		return EncodingProto, fmt.Errorf("unknown encoding %s", v)
	}
}

// String stringifies an Encoding
func (e Encoding) String() string {
	switch {
	case e == EncodingAll:
		return "All"
	case e == EncodingJSON:
		return "JSON"
	case e == EncodingProto:
		return "Proto"
	default:
		return "Encoding" + strconv.FormatInt(int64(e), 10)
	}
}

// DefaultEncoding returns the default kzip encoding
func DefaultEncoding() Encoding {
	if e := os.Getenv("KYTHE_KZIP_ENCODING"); e != "" {
		enc, err := EncodingFor(e)
		if err == nil {
			return enc
		}
		log.Errorf("unknown kzip encoding: %s", e)
	}
	return EncodingProto
}

// A Reader permits reading and scanning compilation records and file contents
// stored in a .kzip archive. The Lookup and Scan methods are mutually safe for
// concurrent use by multiple goroutines.
type Reader struct {
	zip *zip.Reader

	// The archives written by this library always use "root/" for the root
	// directory, but it's not required by the spec. Use whatever name the
	// archive actually specifies in the leading directory.
	root string

	// The prefix used for the compilation unit directory; one of
	// prefixJSON or prefixProto
	unitsPrefix string
}

// NewReader constructs a new Reader that consumes zip data from r, whose total
// size in bytes is given.
func NewReader(r io.ReaderAt, size int64) (*Reader, error) {
	archive, err := zip.NewReader(r, size)
	if err != nil {
		return nil, err
	}
	// Order the files in the archive by path, so we can binary search.
	sort.Slice(archive.File, func(i, j int) bool {
		return archive.File[i].Name < archive.File[j].Name
	})

	if len(archive.File) == 0 {
		return nil, errors.New("archive is empty")
	} else if fi := archive.File[0].FileInfo(); !fi.IsDir() {
		return nil, fmt.Errorf("archive root directory missing: expected a directory but got %v - see https://kythe.io/docs/kythe-kzip.html#_directory_and_file_layout", archive.File[0].Name)
	}
	root := archive.File[0].Name
	pref, err := unitPrefix(root, archive.File)
	if err != nil {
		return nil, err
	}
	return &Reader{
		zip:         archive,
		root:        root,
		unitsPrefix: pref,
	}, nil
}

func unitPrefix(root string, fs []*zip.File) (string, error) {
	jsonDir := root + prefixJSON + "/"
	protoDir := root + prefixProto + "/"
	j := sort.Search(len(fs), func(i int) bool {
		return fs[i].Name > jsonDir
	})
	hasJSON := j < len(fs) && strings.HasPrefix(fs[j].Name, jsonDir)
	p := sort.Search(len(fs), func(i int) bool {
		return fs[i].Name > protoDir
	})
	hasProto := p < len(fs) && strings.HasPrefix(fs[p].Name, protoDir)
	if hasJSON && hasProto {
		// validate that they have identical units based on hash
		for p < len(fs) && j < len(fs) {
			ispb := strings.HasPrefix(fs[p].Name, protoDir)
			isjson := strings.HasPrefix(fs[j].Name, jsonDir)
			if ispb != isjson {
				return "", fmt.Errorf("both proto and JSON units found but are not identical")
			}
			if !ispb {
				break
			}
			pdigest := strings.Split(fs[p].Name, "/")[2]
			jdigest := strings.Split(fs[j].Name, "/")[2]
			if pdigest != jdigest {
				return "", fmt.Errorf("both proto and JSON units found but are not identical")
			}
			p++
			j++
		}
	}
	if hasProto {
		return prefixProto, nil
	}
	return prefixJSON, nil
}

// Encoding exposes the file encoding being used to read compilation units.
func (r *Reader) Encoding() (Encoding, error) {
	switch {
	case r.unitsPrefix == prefixJSON:
		return EncodingJSON, nil
	case r.unitsPrefix == prefixProto:
		return EncodingProto, nil
	}
	return EncodingAll, fmt.Errorf("unknown encoding prefix: %v", r.unitsPrefix)
}

func (r *Reader) unitPath(digest string) string { return path.Join(r.root, r.unitsPrefix, digest) }
func (r *Reader) filePath(digest string) string { return path.Join(r.root, "files", digest) }

// ErrDigestNotFound is returned when a requested compilation unit or file
// digest is not found.
var ErrDigestNotFound = errors.New("digest not found")

// ErrUnitExists is returned by AddUnit when adding the same compilation
// multiple times.
var ErrUnitExists = errors.New("unit already exists")

func (r *Reader) readUnit(digest string, f *zip.File) (*Unit, error) {
	rc, err := f.Open()
	if err != nil {
		return nil, err
	}
	rec := make([]byte, f.UncompressedSize64)
	_, err = io.ReadFull(rc, rec)
	rc.Close()
	if err != nil {
		return nil, err
	}
	var msg apb.IndexedCompilation
	if r.unitsPrefix == prefixProto {
		if err := proto.Unmarshal(rec, &msg); err != nil {
			return nil, fmt.Errorf("error unmarshaling for %s: %s", digest, err)
		}
	} else if err := protojson.Unmarshal(rec, &msg); err != nil {
		return nil, err
	}
	return &Unit{
		Digest: digest,
		Proto:  msg.Unit,
		Index:  msg.Index,
	}, nil
}

// firstIndex returns the first index in the archive's file list whose
// path starts with prefix, or -1 if no such index exists.
func (r *Reader) firstIndex(prefix string) int {
	fs := r.zip.File
	n := sort.Search(len(fs), func(i int) bool {
		return fs[i].Name >= prefix
	})
	if n >= len(fs) {
		return -1
	}
	if !strings.HasPrefix(fs[n].Name, prefix) {
		return -1
	}
	return n
}

// Lookup returns the specified compilation from the archive, if it exists.  If
// the requested digest is not in the archive, ErrDigestNotFound is returned.
func (r *Reader) Lookup(unitDigest string) (*Unit, error) {
	needle := r.unitPath(unitDigest)
	pos := r.firstIndex(needle)
	if pos >= 0 {
		if f := r.zip.File[pos]; f.Name == needle {
			return r.readUnit(unitDigest, f)
		}
	}
	return nil, ErrDigestNotFound
}

// A ScanOption configures the behavior of scanning a kzip file.
type ScanOption interface{ isScanOption() }

type readConcurrency int

func (readConcurrency) isScanOption() {}

// ReadConcurrency returns a ScanOption that configures the max concurrency of
// reading compilation units within a kzip archive.
func ReadConcurrency(n int) ScanOption {
	return readConcurrency(n)
}

func (r *Reader) canonicalUnits() (string, []*zip.File) {
	prefix := r.unitPath("") + "/"
	pos := r.firstIndex(prefix)
	if pos < 0 {
		return "", nil
	}
	var res []*zip.File
	for _, file := range r.zip.File[pos:] {
		if !strings.HasPrefix(file.Name, prefix) {
			break
		}
		if file.Name == prefix {
			continue // tolerate an empty units directory entry
		}
		res = append(res, file)

	}
	return prefix, res
}

// Scan scans all the compilations stored in the archive, and invokes f for
// each compilation record. If f reports an error, the scan is terminated and
// that error is propagated to the caller of Scan.  At most 1 invocation of f
// will occur at any one time.
func (r *Reader) Scan(f func(*Unit) error, opts ...ScanOption) error {
	concurrency := 1
	for _, opt := range opts {
		switch opt := opt.(type) {
		case readConcurrency:
			if n := int(opt); n > 0 {
				concurrency = n
			}
		default:
			return fmt.Errorf("unknown ScanOption type: %T", opt)
		}
	}

	prefix, fileUnits := r.canonicalUnits()
	if len(fileUnits) == 0 {
		return nil
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	g, ctx := errgroup.WithContext(ctx)

	files := make(chan *zip.File)

	g.Go(func() error {
		defer close(files)
		for _, file := range fileUnits {
			select {
			case <-ctx.Done():
				return nil
			case files <- file:
			}
		}
		return nil
	})
	units := make(chan *Unit)
	var wg sync.WaitGroup
	for i := 0; i < concurrency; i++ {
		wg.Add(1)
		g.Go(func() error {
			defer wg.Done()
			for file := range files {
				digest := strings.TrimPrefix(file.Name, prefix)
				unit, err := r.readUnit(digest, file)
				if err != nil {
					return err
				}
				select {
				case <-ctx.Done():
					return nil
				case units <- unit:
				}
			}
			return nil
		})
	}
	go func() { wg.Wait(); close(units) }()
	for unit := range units {
		select {
		case <-ctx.Done():
			return g.Wait()
		default:
			if err := f(unit); err != nil {
				return err
			}
		}
	}
	return g.Wait()
}

// Open opens a reader on the contents of the specified file digest.  If the
// requested digest is not in the archive, ErrDigestNotFound is returned.  The
// caller must close the reader when it is no longer needed.
func (r *Reader) Open(fileDigest string) (io.ReadCloser, error) {
	needle := r.filePath(fileDigest)
	if pos := r.firstIndex(needle); pos >= 0 {
		if f := r.zip.File[pos]; f.Name == needle {
			return f.Open()
		}
	}
	return nil, ErrDigestNotFound
}

// ReadAll returns the complete contents of the file with the specified digest.
// It is a convenience wrapper for Open followed by ioutil.ReadAll.
func (r *Reader) ReadAll(fileDigest string) ([]byte, error) {
	f, err := r.Open(fileDigest)
	if err == nil {
		defer f.Close()
		return ioutil.ReadAll(f)
	}
	return nil, err
}

// A Unit represents a compilation record read from a kzip archive.
type Unit struct {
	Digest string
	Proto  *apb.CompilationUnit
	Index  *apb.IndexedCompilation_Index
}

// A Writer permits construction of a .kzip archive.
type Writer struct {
	mu  sync.Mutex
	zip *zip.Writer
	fd  stringset.Set // file digests already written
	ud  stringset.Set // unit digests already written
	c   io.Closer     // a closer for the underlying writer (may be nil)

	encoding Encoding // What encoding to use
}

// WriterOption describes options when creating a Writer
type WriterOption func(*Writer)

// WithEncoding sets the encoding to be used by a Writer
func WithEncoding(e Encoding) WriterOption {
	return func(w *Writer) {
		w.encoding = e
	}
}

// NewWriter constructs a new empty Writer that delivers output to w.  The
// AddUnit and AddFile methods are safe for use by concurrent goroutines.
func NewWriter(w io.Writer, options ...WriterOption) (*Writer, error) {
	archive := zip.NewWriter(w)
	// Create an entry for the root directory, which must be first.
	root := &zip.FileHeader{
		Name:     "root/",
		Comment:  "kzip root directory",
		Modified: modifiedTime,
	}
	root.SetMode(os.ModeDir | 0755)
	if _, err := archive.CreateHeader(root); err != nil {
		return nil, err
	}
	archive.SetComment("Kythe kzip archive")

	kw := &Writer{
		zip:      archive,
		fd:       stringset.New(),
		ud:       stringset.New(),
		encoding: DefaultEncoding(),
	}
	for _, opt := range options {
		opt(kw)
	}
	return kw, nil
}

// NewWriteCloser behaves as NewWriter, but arranges that when the *Writer is
// closed it also closes wc.
func NewWriteCloser(wc io.WriteCloser, options ...WriterOption) (*Writer, error) {
	w, err := NewWriter(wc, options...)
	if err == nil {
		w.c = wc
	}
	return w, err
}

// toJSON defines the encoding format for compilation messages.
var toJSON = &protojson.MarshalOptions{UseProtoNames: true}

// AddUnit adds a new compilation record to be added to the archive, returning
// the hex-encoded SHA256 digest of the unit's contents. It is legal for index
// to be nil, in which case no index terms will be added.
//
// If the same compilation is added multiple times, AddUnit returns the digest
// of the duplicated compilation along with ErrUnitExists to all callers after
// the first. The existing unit is not modified.
func (w *Writer) AddUnit(cu *apb.CompilationUnit, index *apb.IndexedCompilation_Index) (string, error) {
	unit := kythe.Unit{Proto: cu}
	unit.Canonicalize()
	digest := unit.Digest()

	w.mu.Lock()
	defer w.mu.Unlock()
	if w.ud.Contains(digest) {
		return digest, ErrUnitExists
	}

	if w.encoding&EncodingJSON != 0 {
		f, err := w.zip.CreateHeader(newFileHeader("root", prefixJSON, digest))
		if err != nil {
			return "", err
		}
		rec, err := toJSON.Marshal(&apb.IndexedCompilation{
			Unit:  unit.Proto,
			Index: index,
		})
		if err != nil {
			return "", err
		}
		if _, err := f.Write(rec); err != nil {
			return "", err
		}
	}
	if w.encoding&EncodingProto != 0 {
		f, err := w.zip.CreateHeader(newFileHeader("root", prefixProto, digest))
		if err != nil {
			return "", err
		}
		rec, err := proto.Marshal(&apb.IndexedCompilation{
			Unit:  unit.Proto,
			Index: index,
		})
		if err != nil {
			return "", err
		}
		_, err = f.Write(rec)
		if err != nil {
			return "", err
		}
	}
	w.ud.Add(digest)
	return digest, nil
}

// AddFile copies the complete contents of r into the archive as a new file
// entry, returning the hex-encoded SHA256 digest of the file's contents.
func (w *Writer) AddFile(r io.Reader) (string, error) {
	// Buffer the file contents and compute their digest.
	// We have to do this ahead of time, because we have to provide the name of
	// the file before we can start writing its contents.
	var buf bytes.Buffer
	hash := sha256.New()
	if _, err := io.Copy(io.MultiWriter(hash, &buf), r); err != nil {
		return "", err
	}
	digest := hex.EncodeToString(hash.Sum(nil))

	w.mu.Lock()
	defer w.mu.Unlock()
	if w.fd.Contains(digest) {
		return digest, nil // already written
	}

	f, err := w.zip.CreateHeader(newFileHeader("root", "files", digest))
	if err != nil {
		return "", err
	}
	if _, err := io.Copy(f, &buf); err != nil {
		return "", err
	}
	w.fd.Add(digest)
	return digest, nil
}

// Close closes the writer, flushing any remaining unwritten data out to the
// underlying zip file. It is safe to close w arbitrarily many times; all calls
// after the first will report nil.
func (w *Writer) Close() error {
	w.mu.Lock()
	defer w.mu.Unlock()
	if w.zip != nil {
		err := w.zip.Close()
		w.zip = nil
		if w.c != nil {
			if cerr := w.c.Close(); err == nil {
				return cerr
			}
		}
		return err
	}
	return nil
}

func newFileHeader(parts ...string) *zip.FileHeader {
	fh := &zip.FileHeader{Name: path.Join(parts...), Method: zip.Deflate}
	fh.SetMode(0600)
	fh.Modified = modifiedTime
	return fh
}

// Scan is a convenience function that creates a *Reader from f and invokes its
// Scan method with the given callback. Each invocation of scan is passed the
// reader associated with f, along with the current compilation unit.
func Scan(f File, scan func(*Reader, *Unit) error, opts ...ScanOption) error {
	size, err := f.Seek(0, io.SeekEnd)
	if err != nil {
		return fmt.Errorf("getting file size: %v", err)
	}
	r, err := NewReader(f, size)
	if err != nil {
		return err
	}
	return r.Scan(func(unit *Unit) error {
		return scan(r, unit)
	}, opts...)
}

// A File represents the file capabilities needed to scan a kzip file.
type File interface {
	io.ReaderAt
	io.Seeker
}

// FileData creates a file data protobuf message by fully reading the contents
// of r, having the designated path.
func FileData(path string, r io.Reader) (*apb.FileData, error) {
	var buf bytes.Buffer
	hash := sha256.New()

	w := io.MultiWriter(&buf, hash)
	if _, err := io.Copy(w, r); err != nil {
		return nil, err
	}
	digest := hex.EncodeToString(hash.Sum(nil))
	return &apb.FileData{
		Content: buf.Bytes(),
		Info: &apb.FileInfo{
			Path:   path,
			Digest: digest,
		},
	}, nil
}

// Fetch implements the analysis.Fetcher interface for files attached to c.
// If digest == "", files are matched by path only.
func (c *Compilation) Fetch(path, digest string) ([]byte, error) {
	for _, f := range c.Files {
		info := f.GetInfo()
		fp := info.Path
		fd := info.Digest
		if path == fp && (digest == "" || digest == fd) {
			return f.Content, nil
		}
		if digest != "" && digest == fd {
			return f.Content, nil
		}
	}
	return nil, os.ErrNotExist
}

// Unit returns the CompilationUnit associated with c, creating a new empty one
// if necessary.
func (c *Compilation) Unit() *apb.CompilationUnit {
	if c.Proto == nil {
		c.Proto = new(apb.CompilationUnit)
	}
	return c.Proto
}

// AddFile adds an input file to the compilation by fully reading r.  The file
// is added to the required inputs, attributed to the designated path, and also
// to the file data slice.  If v != nil it is used as the vname of the input
// added.
func (c *Compilation) AddFile(path string, r io.Reader, v *spb.VName, details ...proto.Message) error {
	var anys []*ptypes.Any
	for _, d := range details {
		any, err := ptypes.MarshalAny(d)
		if err != nil {
			return fmt.Errorf("unable to marshal %T to Any: %v", d, err)
		}
		anys = append(anys, any)
	}
	fd, err := FileData(path, r)
	if err != nil {
		return err
	}
	c.Files = append(c.Files, fd)
	unit := c.Unit()
	unit.RequiredInput = append(unit.RequiredInput, &apb.CompilationUnit_FileInput{
		VName:   v,
		Info:    fd.Info,
		Details: anys,
	})
	return nil
}

// AddDetails adds the specified details message to the compilation.
func (c *Compilation) AddDetails(msg proto.Message) error {
	details, err := ptypes.MarshalAny(msg)
	if err != nil {
		return err
	}
	unit := c.Unit()
	unit.Details = append(unit.Details, details)
	return nil
}
