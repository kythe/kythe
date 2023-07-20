/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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

// Package localrun provides helper methods for locally building a Kythe indexed
// repo.
package localrun //import "kythe.io/kythe/go/localrun"

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"
	"time"

	"kythe.io/kythe/go/util/log"

	"github.com/apache/beam/sdks/go/pkg/beam/transforms/stats"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/delimited/dedup"
	"kythe.io/kythe/go/serving/pipeline"
	"kythe.io/kythe/go/serving/pipeline/beamio"
	"kythe.io/kythe/go/serving/xrefs"
	"kythe.io/kythe/go/util/datasize"

	"github.com/apache/beam/sdks/go/pkg/beam"
	"github.com/apache/beam/sdks/go/pkg/beam/x/beamx"

	bespb "kythe.io/third_party/bazel/build_event_stream_go_proto"
)

var _ = delimited.Reader{}
var _ = dedup.Reader{}

// This list must be kept in sync with Language.String.
const (
	lowLanguageMarker Language = iota // Note that lowLanguageMarker must be first.
	Cxx
	Go
	Java
	Jvm
	Protobuf
	Python    // TODO: Python isn't shipping in Kythe right now by default.
	Textproto // TODO: Textproto isn't shipping in Kythe right now by default.
	TypeScript
	highLanguageMarker // Note that highLanguageMarker must be last.
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
	for l := range allLanguages {
		if _, ok := m[l]; !ok {
			panic(fmt.Sprintf("The languageMetadata map needs an entry for %q", l))
		}
	}

	for l := range m {
		if !allLanguages.Has(l) {
			panic(fmt.Sprintf("You've defined a language %v that isn't in AllLanguages()", l))
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
		"lowLanguageMarker",
		"Cxx",
		"Go",
		"Java",
		"Jvm",
		"Protobuf",
		"Python",
		"Textproto",
		"TypeScript",
		"highLanguageMarker",
	}[l]
}

// Valid determines of the provided language is a valid enum value.
func (l Language) Valid() bool {
	return l > lowLanguageMarker && l < highLanguageMarker
}

func (l Language) kZipExtractorName() string {
	return l.metadata().kZipExtractorName
}

func (l Language) indexerPath() string {
	return l.metadata().indexerPath
}

func (l Language) metadata() metadata {
	return languageMetadataMap[l]
}

// AllLanguages returns a language set that contains all available languages.
func AllLanguages() LanguageSet {
	languages := LanguageSet{}
	for c := Language(lowLanguageMarker + 1); c.Valid(); c++ {
		languages.Set(c)
	}
	return languages
}

// LanguageSet is a set implementation that tracks langages.
type LanguageSet map[Language]struct{}

// String implements Stringer.String.
func (ls LanguageSet) String() string {
	languages := []string{}
	for l := range ls {
		languages = append(languages, l.String())
	}
	return strings.Join(languages, ",")
}

// Set the provided language in the set.
func (ls LanguageSet) Set(l Language) {
	ls[l] = struct{}{}
}

// Has checks if the provided language is in the set.
func (ls LanguageSet) Has(l Language) bool {
	_, ok := ls[l]
	return ok
}

func (ls LanguageSet) hasExtractor(e string) bool {
	for l := range ls {
		if l.kZipExtractorName() == e {
			return true
		}
	}
	return false
}

// LanguageMap is a mapping from the string name of each language to the enum
// value.
var LanguageMap map[string]Language = makeLanguageMap()

func makeLanguageMap() map[string]Language {
	r := map[string]Language{}
	for l := range AllLanguages() {
		r[strings.ToLower(l.String())] = l
	}
	return r
}

func KnownLanguage(s string) (Language, bool) {
	l, ok := LanguageMap[s]
	return l, ok
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

// Runner is responsible for indexing the repo with Kythe.
type Runner struct {
	// Generally useful configuration
	KytheRelease   string
	WorkingDir     string
	OutputDir      string
	WorkerPoolSize int
	CacheSize      *datasize.Size

	// Building/extracting options.
	Languages LanguageSet
	Targets   []string

	// Serving configuration options.
	Port            int
	Hostname        string
	PublicResources string

	// Timeout for indexing.
	Timeout time.Duration

	// Internal state
	m          mode
	indexedOut io.Reader
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
	log.Infof("Building with event file at: %s", r.besFile)

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

	var kzips []string

	file, err := os.Open(r.besFile) // For read access.
	if err != nil {
		return fmt.Errorf("unable to open build event stream file: %v", err)
	}

	rd := delimited.NewReader(file)

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
			log.Warningf("Didn't find extractor for: %s", action.Type)
			continue
		}

		actionCompleted := event.Id.GetActionCompleted()
		if actionCompleted == nil {
			continue
		}

		kzips = append(kzips, actionCompleted.PrimaryOutput)
	}

	log.Info("Finished finding indexable targets")

	indexer := &serialIndexer{
		besFile:      r.besFile,
		Languages:    r.Languages,
		KytheRelease: r.KytheRelease,
		WorkingDir:   r.WorkingDir,
		Timeout:      r.Timeout,
	}

	r.indexedOut, err = indexer.run(ctx, kzips)
	return err
}

// PostProcess postprocesses the supplied indexed data.
func (r *Runner) PostProcess(ctx context.Context) error {
	if err := r.checkMode(postprocess); err != nil {
		return err
	}

	log.Info("Starting postprocessing")

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

	opts := stats.Opts{
		NumQuantiles: r.WorkerPoolSize,
		K:            r.WorkerPoolSize,
	}
	beamio.WriteLevelDB(s, r.OutputDir, opts,
		createColumnarMetadata(s),
		k.SplitCrossReferences(),
		k.SplitDecorations(),
		k.CorpusRoots(),
		k.Directories(),
		k.Documents(),
		k.SplitEdges(),
	)

	return beamx.Run(ctx, p)

	// dedup the stream
	// https://github.com/kythe/kythe/blob/77e1360beca7903bc97728097d2c5e1b82204092/kythe/go/platform/tools/dedup_stream/dedup_stream.go
	// https://github.com/kythe/kythe/blob/77e1360beca7903bc97728097d2c5e1b82204092/kythe/go/serving/tools/write_tables/write_tables.go#L139
}

func createColumnarMetadata(s beam.Scope) beam.PCollection {
	return beam.ParDo(s, emitColumnarMetadata, beam.Impulse(s))
}

func emitColumnarMetadata(_ []byte) (string, string) { return xrefs.ColumnarTableKeyMarker, "v1" }

// Serve starts an http server on the provided port and allows web inspection
// of the indexed data.
func (r *Runner) Serve(ctx context.Context) error {
	if err := r.checkMode(serve); err != nil {
		return err
	}

	listen := fmt.Sprintf("%s:%d", r.Hostname, r.Port)

	args := []string{
		"--listen", listen,
		"--serving_table", r.OutputDir,
	}

	if _, err := os.Stat(r.PublicResources); !os.IsNotExist(err) {
		args = append(args, "--public_resources")
		args = append(args, r.PublicResources)
	} else if r.PublicResources != "" {
		log.Warningf("You requested serving public resources from %q, but it doesn't exist.", r.PublicResources)
	}

	// Perform the work prescribed
	cmd := exec.CommandContext(ctx, fmt.Sprintf("%s/tools/http_server", r.KytheRelease), args...)
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout
	cmd.Dir = r.WorkingDir

	log.Infof("Starting http server on http://%s", listen)
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("error starting serve: %w", err)
	}
	return nil
}

type serialIndexer struct {
	besFile      string
	Languages    LanguageSet
	KytheRelease string
	WorkingDir   string
	Timeout      time.Duration
}

func (si *serialIndexer) run(ctx context.Context, kzips []string) (io.Reader, error) {
	indexedOut := &bytes.Buffer{}

	log.Info("Beginning serial indexing")

	// TODO: Dedup the kzips instead of keeping them all around.
	for i, k := range kzips {
		log := func(i int, m string, v ...any) {
			den := len(kzips)
			log.Infof("[%d/%d] %v", i, den, fmt.Sprintf(m, v...))
		}

		log(i, "Started indexing %q", k)

		if f, err := os.Stat(k); err != nil {
			log(i, "file error: %v", err)
		} else if f.Size() == 0 {
			log(i, "skipping empty .kzip")
			continue
		}

		parts := strings.Split(k, ".")
		langStr := parts[len(parts)-2]
		l, ok := LanguageMap[langStr]
		if !ok {
			log(i, "Unrecognized language")
			return indexedOut, fmt.Errorf("unrecognized language: %v", langStr)
		}

		cmdCtx, cancel := context.WithTimeout(ctx, si.Timeout)
		defer cancel()

		// Perform the work prescribed
		cmd := exec.CommandContext(cmdCtx,
			fmt.Sprintf("%s/indexers/%s", si.KytheRelease, l.indexerPath()),
			k,
		)

		cmd.Stdout = indexedOut
		cmd.Stderr = os.Stderr
		cmd.Dir = si.WorkingDir

		if err := cmd.Run(); err != nil {
			log(i, "Failed run: %v", cmd.Args)
			return indexedOut, fmt.Errorf("error indexing[%v] %q: %v", l.String(), k, err)
		}
	}
	log.Infof("\n[%d/%d] Finished indexing", len(kzips), len(kzips))

	return indexedOut, nil
}
