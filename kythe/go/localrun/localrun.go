package localrun

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/x/beamx"

	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/delimited/dedup"
	"kythe.io/kythe/go/services/graphstore"
	"kythe.io/kythe/go/serving/pipeline"
	"kythe.io/kythe/go/serving/pipeline/beamio"
	"kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/util/datasize"
)

var _ = delimited.Reader{}
var _ = dedup.Reader{}

const (
	Cxx Language = iota
	Go
	Java
	Jvm
	Protobuf
	Python
	TypeScript // Note that typescript must be last, or you must update Valid to refer to the last element.
)

// Language defines
type Language int

// String provides the string representation of this enum value.
func (l Language) String() string {
	return [...]string{
		"Cxx",
		"Go",
		"Java",
		"Jvm",
		"Protobuf",
		"Python",
		"TypeScript",
	}[l]
}

// Valid determines of the provided language is a valid enum value.
func (l Language) Valid() bool {
	return l <= TypeScript
}

var LanguageMap map[string]Language = makeLanguageMap()

func makeLanguageMap() map[string]Language {
	r := map[string]Language{}
	for c := Language(0); c.Valid(); c++ {
		r[strings.ToLower(c.String())] = c
	}
	return r
}

const (
	// Ordered linear set of states the localrun program can be in.
	nothing mode = iota
	extract
	index
	postprocess
	serve
)

type mode int

func (m mode) String() string {
	return [...]string{"nothing", "extract", "index", "postprocess", "serve"}[m]
}

type Runner struct {
	// Generally useful configuration
	KytheRelease   string
	WorkingDir     string
	OutputDir      string
	WorkerPoolSize int
	CacheSize      *datasize.Size
	graphStore     graphstore.Service

	// Building/extracting options.
	Languages []Language
	Targets   []string

	// Serving configuration options.
	Port int

	// Internal state
	m          mode
	indexedOut *threadsafeBuffer
}

func (r *Runner) checkMode(m mode) error {
	if r.m != m-1 {
		return fmt.Errorf("couldn't transition to state %q from mode %q", m, r.m)
	}

	r.m = m
	return nil
}

// Extract builds the provided targets and extracts compilation results.
func (r *Runner) Extract(ctx context.Context) error {
	if err := r.checkMode(extract); err != nil {
		return err
	}

	// TODO: This is just a simple translation of the compile script into
	args := append([]string{
		fmt.Sprintf("--bazelrc=%s/extractors.bazelrc", r.KytheRelease),
		"build",
		"--keep_going",
		// TODO: Is this a good idea? It makes it a lot easier to
		// develop this since you can see successes/failures but may
		// break CI workflows.
		"--color=yes",
		fmt.Sprintf("--override_repository=kythe_release=%s", r.KytheRelease),
		"--",
	},
		r.Targets...)

	cmd := exec.CommandContext(ctx,
		"bazel",
		args...)
	cmd.Stderr = os.Stderr
	cmd.Dir = r.WorkingDir

	if err := cmd.Run(); err != nil {
		return fmt.Errorf("error: %v", err)
	}

	return nil
}

// Index indexes the extracted data.
// Index is a reimplementation of the https://github.com/kythe/kythe/blob/master/kythe/release/kythe.sh script's --index mode.
func (r *Runner) Index(ctx context.Context) error {
	if err := r.checkMode(index); err != nil {
		return err
	}

	// TODO: one segment of this path is "k8-fastbuild" which is not always
	// correct.  filepath.Walk does not follow symlinks (which is the only
	// thing that Bazel seems to make). Hardcode to a dir inside so we have
	// something to traverse and come back later.
	path := filepath.Join(r.WorkingDir, "bazel-out", "k8-fastbuild", "extra_actions")

	g, _ := errgroup.WithContext(ctx)

	kzips := make(chan string, r.WorkerPoolSize)

	var countDenom uint64
	g.Go(func() error {
		defer close(kzips)

		if err := filepath.Walk(path, func(path string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}

			// TODO: Don't filter out non-go
			//if !strings.HasSuffix(path, ".kzip") {
			if !strings.HasSuffix(path, ".go.kzip") {
				return nil
			}

			atomic.AddUint64(&countDenom, 1)
			kzips <- path

			// Create parallel work entries that need to be performed.
			return nil
		}); err != nil {
			return fmt.Errorf("error walking %q: %v", path, err)
		}

		fmt.Fprintf(os.Stderr, "Finished walking tree\n")
		return nil
	})

	fmt.Fprintf(os.Stderr, "Beginning indexing\n")

	r.indexedOut = &threadsafeBuffer{}

	var countNum uint64
	for workerID := 0; workerID < r.WorkerPoolSize; workerID++ {
		g.Go(func(workerID int) func() error {
			return func() error {
				fmt.Fprintf(os.Stderr, "Parallel indexing worker %d started\n", workerID)

				workerBuf := &bytes.Buffer{}

				for k := range kzips {
					log := func(c uint64, m string, v ...interface{}) {
						den := atomic.LoadUint64(&countDenom)
						fmt.Fprintf(os.Stderr, "[%d/%d] %v \n", c, den, fmt.Sprintf(m, v...))
					}

					c := atomic.AddUint64(&countNum, 1)
					log(c, "Started indexing %q", k)

					if f, err := os.Stat(k); err != nil {
						log(c, "file error: %v", err)
					} else if f.Size() == 0 {
						log(c, "skipping empty .kzip")
						continue
					}

					parts := strings.Split(k, ".")
					langStr := parts[len(parts)-2]
					l, ok := LanguageMap[langStr]
					if !ok {
						log(c, "Unrecognized language")
						return fmt.Errorf("Unrecognized language: %v\n", langStr)
					}

					cmdCtx, cancel := context.WithTimeout(ctx, 5*time.Second)
					defer cancel()

					// Perform the work prescribed
					cmd := exec.CommandContext(cmdCtx,
						fmt.Sprintf("/opt/kythe/indexers/%s_indexer", strings.ToLower(l.String())),
						//"-continue",
						//"-json",
						k)

					cmd.Stdout = workerBuf
					cmd.Stderr = os.Stderr
					cmd.Dir = r.WorkingDir

					if err := cmd.Run(); err != nil {
						log(c, "Failed run: %v", cmd.Args)
						return fmt.Errorf("error indexing[%v] %q: %v", l.String(), k, err)
					}
				}
				fmt.Fprintf(os.Stderr, "Parallel indexing worker %d completed\n", workerID)

				if _, err := workerBuf.WriteTo(r.indexedOut); err != nil {
					return err
				}

				return nil
			}
		}(workerID))
	}

	if err := errGroupWait(g, "indexing workers"); err != nil {
		return err
	}
	fmt.Fprintf(os.Stderr, "\n[%d/%d] Finished indexing\n", countNum, countDenom)

	return nil
}

// PostProcess postprocesses the supplied indexed data.
func (r *Runner) PostProcess(ctx context.Context) error {
	if err := r.checkMode(postprocess); err != nil {
		return err
	}

	fmt.Fprintf(os.Stderr, "\nStarting postprocessing\n")

	rd, err := dedup.NewReader(r.indexedOut, int(r.CacheSize.Bytes()))
	if err != nil {
		return fmt.Errorf("error creating deduped stream: %v", err)
	}

	tmpfile, err := ioutil.TempFile("", "dedup_kythe_stream.pb")
	if err != nil {
		return fmt.Errorf("error creating tmpfile: %v", err)
	}
	defer os.Remove(tmpfile.Name())

	wr := delimited.NewWriter(tmpfile)
	if err := delimited.Copy(wr, rd); err != nil {
		return fmt.Errorf("unable to copy deduped stream to disk: %v", err)
	}

	p, s := beam.NewPipelineWithRoot()
	entries, err := beamio.ReadEntries(ctx, s, tmpfile.Name())
	if err != nil {
		return fmt.Errorf("error reading entries: %v", err)
	}
	k := pipeline.FromEntries(s, entries)

	beamio.WriteLevelDB(s, r.OutputDir, r.WorkerPoolSize,
		createColumnarMetadata(s),
		k.SplitCrossReferences(),
		k.SplitDecorations(),
		k.CorpusRoots(),
		k.Directories(),
		k.Documents(),
		k.SplitEdges(),
	)

	g, _ := errgroup.WithContext(ctx)
	g.Go(func() error {
		return beamx.Run(ctx, p)
	})
	return errGroupWait(g, "postprocessing pipeline")

	// dedup the stream
	// https://github.com/kythe/kythe/blob/77e1360beca7903bc97728097d2c5e1b82204092/kythe/go/platform/tools/dedup_stream/dedup_stream.go
	// https://github.com/kythe/kythe/blob/77e1360beca7903bc97728097d2c5e1b82204092/kythe/go/serving/tools/write_tables/write_tables.go#L139
}

func createColumnarMetadata(s beam.Scope) beam.PCollection {
	return beam.ParDo(s, emitColumnarMetadata, beam.Impulse(s))
}

func emitColumnarMetadata(_ []byte) (string, string) { return xrefs.ColumnarTableKeyMarker, "v1" }

func errGroupWait(g *errgroup.Group, msg string) error {
	done := make(chan error, 1)
	go func() {
		fmt.Fprintf(os.Stderr, "Waiting on %s...\n", msg)
		done <- g.Wait()
		fmt.Fprintf(os.Stderr, "Done waiting on %s...\n", msg)
	}()

	for {
		select {
		case err := <-done:
			if err != nil {
				return fmt.Errorf("errorGroup had an error: %v", err)
			}
			return nil
		case <-time.After(1 * time.Second):
			fmt.Fprintf(os.Stderr, ".")
		}
	}
}

// Serve starts an http server on the provided port and allows web inspection
// of the indexed data.
func (r *Runner) Serve(ctx context.Context) error {
	if err := r.checkMode(serve); err != nil {
		return err
	}

	listen := fmt.Sprintf(":%d", r.Port)

	// Perform the work prescribed
	cmd := exec.CommandContext(ctx,
		fmt.Sprintf("%s/tools/http_server", r.KytheRelease),
		"--listen", listen,
		"--serving_table", r.OutputDir,
		"--public_resources", "/usr/local/google/home/achew/kythe/kythe/web/ui/resources/public",
	)
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout
	cmd.Dir = r.WorkingDir

	fmt.Fprintf(os.Stderr, "Starting http server on http://%s\n", listen)
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("error starting dedup: %v", err)
	}
	return nil
}

// threadsafeBuffer implements a threadsafe form of bytes.Buffer
type threadsafeBuffer struct {
	b bytes.Buffer
	m sync.RWMutex
}

// Read implements Buffer.Read.
func (b *threadsafeBuffer) Read(p []byte) (n int, err error) {
	b.m.RLock()
	defer b.m.RUnlock()
	return b.b.Read(p)
}

// Write implements Buffer.Write.
func (b *threadsafeBuffer) Write(p []byte) (n int, err error) {
	b.m.Lock()
	defer b.m.Unlock()
	return b.b.Write(p)
}

// String implements Buffer.String.
func (b *threadsafeBuffer) String() string {
	b.m.RLock()
	defer b.m.RUnlock()
	return b.b.String()
}

// WriteTo implements Buffer.WriteTo.
func (b *threadsafeBuffer) WriteTo(w io.Writer) (int64, error) {
	b.m.RLock()
	defer b.m.RUnlock()
	return b.b.WriteTo(w)
}
