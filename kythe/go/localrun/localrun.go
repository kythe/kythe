package localrun

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"sync/atomic"
	"time"

	"golang.org/x/sync/errgroup"
)

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

	// Building/extracting options.
	Languages []Language
	Targets   []string

	// Serving configuration options.
	Port int

	// Internal state
	m mode
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

	indexedOut := &bytes.Buffer{}

	var countNum uint64
	//for workerId := 0; workerId < r.WorkerPoolSize; workerId++ {
	for workerId := 0; workerId < 1; workerId++ {
		g.Go(func() error {
			// Capture the workerId so that you can see
			workerId := workerId
			fmt.Fprintf(os.Stderr, "Worker %d started\n", workerId)

			for k := range kzips {
				log := func(c uint64, m string, v ...interface{}) {
					den := atomic.LoadUint64(&countDenom)
					fmt.Fprintf(os.Stderr, "%v %v [%d/%d Worker id: %d]\n", fmt.Sprintf(m, v...), k, c, den, workerId)
				}

				c := atomic.AddUint64(&countNum, 1)
				log(c, "Started indexing")

				parts := strings.Split(k, ".")
				langStr := parts[len(parts)-2]
				l, ok := LanguageMap[langStr]
				if !ok {
					fmt.Fprintf(os.Stderr, "Unrecognized language: %v\n", langStr)
					log(c, "Started indexing")
					return fmt.Errorf("Unrecognized language: %v\n", langStr)
				}

				cmdCtx, cancel := context.WithTimeout(ctx, 5*time.Second)
				defer cancel()

				// Perform the work prescribed
				cmd := exec.CommandContext(cmdCtx,
					fmt.Sprintf("/opt/kythe/indexers/%s_indexer", strings.ToLower(l.String())),
					k)
				cmd.Stdout = indexedOut
				cmd.Stderr = os.Stderr
				cmd.Dir = r.WorkingDir

				log(c, "Running")
				if err := cmd.Run(); err != nil {
					log(c, "Failed run")
					return fmt.Errorf("error indexing[%v] %q: %v", l.String(), k, err)
				}
				log(c, "Done running")

			}
			fmt.Fprintf(os.Stderr, "Worker %d complete\n", workerId)
			return nil
		})
	}

	gCtxDone := make(chan struct{})
	go func() {
		fmt.Fprintf(os.Stderr, "Waiting on group...\n\n\n\n")
		g.Wait()
		fmt.Fprintf(os.Stderr, "Done waiting on group...\n\n\n\n")
		gCtxDone <- struct{}{}
	}()
dotloop:
	for {
		select {
		case <-gCtxDone:
			if err := g.Wait(); err != nil {
				return err
			}
			break dotloop
		case <-time.After(1 * time.Second):
			fmt.Fprintf(os.Stderr, ".")
		}
	}
	fmt.Fprintf(os.Stderr, "\nFinished indexing\n")

	// Create a write_entries command to write to disk the result of indexing.
	writeCmd := exec.CommandContext(ctx,
		fmt.Sprintf("%s/tools/write_entries", r.KytheRelease),
		"-workers", strconv.Itoa(r.WorkerPoolSize),
		"-graphstore", r.OutputDir,
	)
	writeStdin, err := writeCmd.StdinPipe()
	if err != nil {
		return fmt.Errorf("Unable to make write_entries stdin pipe: %v", err)
	}
	writeCmd.Stderr = os.Stderr
	writeCmd.Stdout = os.Stdout

	if err := writeCmd.Start(); err != nil {
		return fmt.Errorf("error starting write: %v", err)
	}

	// Perform the work prescribed
	dedupCmd := exec.CommandContext(ctx,
		fmt.Sprintf("%s/tools/dedup_stream", r.KytheRelease))
	dedupW, err := dedupCmd.StdinPipe()
	if err != nil {
		return fmt.Errorf("unable to make dedup stdinpipe: %v", err)
	}
	dedupCmd.Stderr = os.Stderr
	dedupCmd.Stdout = writeStdin
	dedupCmd.Dir = r.WorkingDir

	if err := dedupCmd.Start(); err != nil {
		return fmt.Errorf("error starting dedup: %v", err)
	}

	if _, err := indexedOut.WriteTo(dedupW); err != nil {
		return fmt.Errorf("Unable to write to deduped write buffer: %v", err)
	}

	if err := dedupW.Close(); err != nil {
		return err
	}

	if err := dedupCmd.Wait(); err != nil {
		return fmt.Errorf("error in dedup wait: %v", err)
	}
	fmt.Fprintf(os.Stderr, "Finished dedup\n")

	if err := writeCmd.Wait(); err != nil {
		return fmt.Errorf("error in write_entries: %v", err)
	}
	fmt.Fprintf(os.Stderr, "Finished write_entries\n")

	// dedup the stream
	// https://github.com/kythe/kythe/blob/77e1360beca7903bc97728097d2c5e1b82204092/kythe/go/platform/tools/dedup_stream/dedup_stream.go
	return nil
}

// Postprocess postprocesses the supplied indexed data.
func (r *Runner) Postprocess(ctx context.Context) error {
	if err := r.checkMode(postprocess); err != nil {
		return err
	}

	// https://github.com/kythe/kythe/blob/77e1360beca7903bc97728097d2c5e1b82204092/kythe/go/serving/tools/write_tables/write_tables.go#L139
	return nil
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
	)
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout
	cmd.Dir = r.WorkingDir

	fmt.Fprintf(os.Stderr, "Starting http server on http://%s", listen)
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("error starting dedup: %v", err)
	}
	return nil
}
