package localrun

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
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

	bespb "kythe.io/third_party/bazel/build_event_stream_go_proto"
)

var _ = delimited.Reader{}
var _ = dedup.Reader{}

const (
	Cxx Language = iota
	Go
	Java
	Jvm
	Protobuf
	// TODO: Python isn't shipping in kythe right now by default
	Python
	Textproto
	// TODO: TypeScript isn't shipping in kythe right now by default.
	TypeScript // Note that typescript must be last, or you must update Valid to refer to the last element.
)

type metadata struct {
	kZipExtractorName string
	indexerPath       string
}

var languageMetadataMap = makeLanguageMetadata()

func makeLanguageMetadata() map[Language]metadata {
	m := map[Language]metadata{
		Cxx: metadata{
			kZipExtractorName: "extract_kzip_cxx_extra_action",
			indexerPath:       "cxx_indexer",
		},
		Go: metadata{
			kZipExtractorName: "extract_kzip_go_extra_action",
			indexerPath:       "go_indexer",
		},
		Java: metadata{
			kZipExtractorName: "extract_kzip_java_extra_action",
			indexerPath:       "java_indexer.jar",
		},
		Jvm: metadata{
			kZipExtractorName: "extract_kzip_jvm_extra_action",
			indexerPath:       "jvm_indexer.jar",
		},
		Protobuf: metadata{
			kZipExtractorName: "extract_kzip_protobuf_extra_action",
			indexerPath:       "proto_indexer",
		},
		Python: metadata{
			kZipExtractorName: "extract_kzip_python_extra_action",
			indexerPath:       "python_indexer",
		},
		Textproto: metadata{
			kZipExtractorName: "extract_kzip_protobuf_extra_action",
			indexerPath:       "textproto_indexer",
		},
		TypeScript: metadata{
			kZipExtractorName: "extract_kzip_typeScript_extra_action",
			indexerPath:       "typescript_indexer",
		},
	}

	allLanguages := AllLanguages()
	for _, l := range allLanguages {
		if _, ok := m[l]; !ok {
			panic(fmt.Sprintf("The languageMetadata map needs an entry for %q", l))
		}
	}

	for l := range m {
		if !allLanguages.contains(l) {
			panic(fmt.Sprintf("You've defined a language %q that isn't in AllLanguages()", l))
		}
	}

	if len(m) != len(AllLanguages()) {
		panic(fmt.Sprintf("The languageMetadata map should have an entry for every language"))
	}
	return m
}

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

func (l Language) kZipExtractorName() string {
	return l.metadata().kZipExtractorName
}

func (l Language) indexerPath() string {
	return l.metadata().indexerPath
}

// Valid determines of the provided language is a valid enum value.
func (l Language) Valid() bool {
	return l <= TypeScript
}

func (l Language) metadata() metadata {
	return languageMetadataMap[l]
}

func AllLanguages() LanguageList {
	languages := LanguageList{}
	for c := Language(0); c.Valid(); c++ {
		languages = append(languages, c)
	}
	return languages
}

type LanguageList []Language

func (ll LanguageList) String() string {
	languages := []string{}
	for _, l := range ll {
		languages = append(languages, l.String())
	}
	return strings.Join(languages, ",")
}

func (ll LanguageList) contains(l Language) bool {
	for _, v := range ll {
		if v == l {
			return true
		}
	}
	return false
}

func (ll LanguageList) hasExtractor(e string) bool {
	for _, v := range ll {
		if v.kZipExtractorName() == e {
			return true
		}
	}
	return false
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
	Languages LanguageList
	Targets   []string

	// Serving configuration options.
	Port int

	// Internal state
	m          mode
	indexedOut *threadsafeBuffer
	besFile    string
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

	tmpfile, err := ioutil.TempFile("", "bazel_build_event_protocol.pb")
	if err != nil {
		return fmt.Errorf("error creating tmpfile: %v", err)
	}
	r.besFile = tmpfile.Name()

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
		fmt.Sprintf("--build_event_binary_file=%s", r.besFile),
		"--",
	},
		r.Targets...)
	fmt.Fprintf(os.Stderr, "Building with event file at: %s\n", r.besFile)

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

	g, _ := errgroup.WithContext(ctx)

	file, err := os.Open(r.besFile) // For read access.
	if err != nil {
		return fmt.Errorf("Unable to open build event stream file: %v", err)
	}

	rd := delimited.NewReader(file)

	kzips := make(chan string, r.WorkerPoolSize)

	var countDenom uint64
	g.Go(func() error {
		defer close(kzips)

		for {
			var event bespb.BuildEvent
			if err := rd.NextProto(&event); err == io.EOF {
				break
			} else if err != nil {
				return fmt.Errorf("error reading length-delimited proto from bes file: %v", err)
			}

			action := event.GetAction()
			if action == nil {
				continue
			}

			if !action.Success {
				continue
			}
			if !r.Languages.hasExtractor(action.Type) {
				fmt.Sprintf("Didn't find extractor for: %s\n", action.Type)
				continue
			}

			actionCompleted := event.Id.GetActionCompleted()
			if actionCompleted == nil {
				continue
			}

			atomic.AddUint64(&countDenom, 1)
			kzips <- actionCompleted.PrimaryOutput
		}

		fmt.Fprintf(os.Stderr, "Finished finding indexable targets\n")
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

					cmdCtx, cancel := context.WithTimeout(ctx, 300*time.Second)
					defer cancel()

					// Perform the work prescribed
					cmd := exec.CommandContext(cmdCtx,
						fmt.Sprintf("%s/indexers/%s", r.KytheRelease, l.indexerPath()),
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
