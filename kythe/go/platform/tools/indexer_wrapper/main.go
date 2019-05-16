package main

// This binary reads compilation units from the provided kzip(s) and indexes
// them using language-specific indexers. Entry protos are written to stdout.
//
// TODO(https://github.com/kythe/kythe/pull/3339): after the analyzer driver
// interface is finalized, update this binary to use it rather than creating a
// temporary kzip file for each indexer invocation.

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"

	"kythe.io/kythe/go/platform/kzip"
)

// saveSingleUnitTmpKzip creates a new kzip containing only the given compilation unit.
// File contents are read from the given kzip.Reader object.
func saveSingleUnitTmpKzip(r *kzip.Reader, cu *kzip.Unit) (string, error) {
	tmpfile, err := ioutil.TempFile("", fmt.Sprintf("*_%s.kzip", cu.Proto.VName.Language))
	if err != nil {
		return "", err
	}

	w, err := kzip.NewWriter(tmpfile)
	if err != nil {
		return "", err
	}
	defer w.Close()

	if _, err = w.AddUnit(cu.Proto, cu.Index); err != nil {
		return "", err
	}

	// Copy all required files into the kzip.
	for _, file := range cu.Proto.RequiredInput {
		f, err := r.Open(file.Info.Digest)
		if err != nil {
			return "", err
		}
		defer f.Close()

		if _, err = w.AddFile(f); err != nil {
			return "", err
		}
	}

	return tmpfile.Name(), nil
}

// binPath returns the absolute path to an indexer given its name.
func binPath(binname string) string {
	return filepath.Join(*releaseDir, "indexers", binname)
}

// indexerCommand returns the command/args to index a kzip of a particular
// language.
func indexerCommand(path string, lang string) ([]string, error) {
	switch lang {
	case "c++":
		return []string{binPath("cxx_indexer"), path}, nil
	case "java":
		return []string{"java", "-jar", binPath("java_indexer.jar"), path}, nil
	case "jvm":
		return []string{"java", "-jar", binPath("jvm_indexer.jar"), path}, nil
	case "protobuf":
		return []string{binPath("proto_indexer"), "--index_file", path}, nil
	case "textproto":
		return []string{binPath("textproto_indexer"), "--index_file", path}, nil
	case "go":
		return []string{binPath("go_indexer"), path}, nil
	default:
		return nil, fmt.Errorf("unrecognized language: %q", lang)
	}
}

// indexMultiLanguageKzip indexes all compilation units containined in the given
// kzip. A temporary kzip is created for each individual compilation unit and
// passed to the corresponding language-specific indexer.
func indexMultiLanguageKzip(path string, out *os.File) error {
	// Read kzip.
	f, err := os.Open(path)
	if err != nil {
		return fmt.Errorf("opening kzip %s: %v", path, err)
	}
	fi, err := f.Stat()
	if err != nil {
		return fmt.Errorf("getting kzip size %s: %v", path, err)
	}
	r, err := kzip.NewReader(f, fi.Size())
	if err != nil {
		return fmt.Errorf("opening kzip reader %s: %v", path, err)
	}

	// For each compilation unit in the kzip, save a temporary kzip containing
	// only that unit and run the appropriate indexer on it.
	if err := r.Scan(func(cu *kzip.Unit) error {
		if cu.Proto == nil {
			return nil
		}

		tmpfilepath, err := saveSingleUnitTmpKzip(r, cu)
		if err != nil {
			return fmt.Errorf("creating tmp kzip: %v", err)
		}
		defer os.Remove(tmpfilepath)

		// Run language-specific indexer.
		args, err := indexerCommand(tmpfilepath, cu.Proto.VName.Language)
		if err != nil {
			return err
		}
		cmd := exec.Command(args[0], args[1:]...)
		cmd.Stdout = out
		cmd.Stderr = os.Stderr
		if err = cmd.Run(); err != nil {
			return fmt.Errorf("indexing kzip %s: %v", tmpfilepath, err)
		}
		return nil
	}, kzip.ReadConcurrency(*concurrency)); err != nil {
		return fmt.Errorf("scanning kzip %s: %v", path, err)
	}

	return nil
}

var (
	concurrency = flag.Int("concurrency", 8, "How many compilation units to process in parallel")
	releaseDir  = flag.String("release_dir", "/opt/kythe", "Path to kythe release dir, which contains indexer binaries and more")
)

func init() {
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s file.kzip...

Indexes all compilation units from the provided kzip(s) by delegating to
language-specific indexers. Entry protos are written to stdout.

Options:
`, filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
}

func main() {
	flag.Parse()
	if flag.NArg() == 0 {
		flag.Usage()
		os.Exit(1)
	}

	for _, path := range flag.Args() {
		if err := indexMultiLanguageKzip(path, os.Stdout); err != nil {
			log.Fatalf("indexing failed for %s: %v", path, err)
		}
	}
}
