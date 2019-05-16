// This package provides an implementation of the CompilationAnalyzer interface
// that works by saving the compilation unit to a temporary kzip file and
// invoking an indexer binary on it.
package main

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"

	"kythe.io/kythe/go/platform/analysis"
	"kythe.io/kythe/go/platform/delimited"
	"kythe.io/kythe/go/platform/kzip"
	apb "kythe.io/kythe/proto/analysis_go_proto"
)

type localAnalyzer struct {
	cmd     []string
	fetcher analysis.Fetcher
}

// NewLocalAnalyzer creates an analyzer that will execute the given command when
// Analyze() is called. The filepath to a kzip is appended to the command, so if
// given ["my_indexer", "--index"], the final executed command will be
// ["my_indexer", "--index", "some/file.kzip"].
func NewLocalAnalyzer(cmd []string, fetcher analysis.Fetcher) *localAnalyzer {
	return &localAnalyzer{cmd: cmd, fetcher: fetcher}
}

func (a *localAnalyzer) Analyze(ctx context.Context, req *apb.AnalysisRequest, f analysis.OutputFunc) error {
	tmpkzip, err := saveSingleUnitKzip(ctx, req.Compilation, a.fetcher)
	if err != nil {
		return fmt.Errorf("creating tmp kzip: %v", err)
	}
	defer os.Remove(tmpkzip)

	// Run indexer binary
	var errBuf strings.Builder
	var outBuf bytes.Buffer
	cmd := exec.Command(a.cmd[0], append(a.cmd[1:], tmpkzip)...)
	cmd.Stdout = &outBuf
	cmd.Stderr = &errBuf
	if err = cmd.Run(); err != nil {
		return fmt.Errorf("indexing kzip %s: %v. stderr=%s", tmpkzip, err, errBuf.String())
	}

	// Deserialize output and pass it to `f`.
	rd := delimited.NewReader(&outBuf)
	for {
		n, err := rd.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return fmt.Errorf("parsing entry stream: %v", err)
		}

		f(ctx, &apb.AnalysisOutput{Value: n})
	}
	f(ctx, &apb.AnalysisOutput{FinalResult: &apb.AnalysisResult{Status: apb.AnalysisResult_COMPLETE}})

	return nil
}

// saveSingleUnitKzip creates a kzip file containing the given compilation unit
// and any files it depends on. File contents are retrieved from the given
// fetcher object.
func saveSingleUnitKzip(ctx context.Context, cu *apb.CompilationUnit, fetcher analysis.Fetcher) (string, error) {
	tmpfile, err := ioutil.TempFile("", fmt.Sprintf("*_%s.kzip", cu.VName.Language))
	if err != nil {
		return "", err
	}

	w, err := kzip.NewWriter(tmpfile)
	if err != nil {
		return "", err
	}
	defer w.Close()

	if _, err = w.AddUnit(cu, nil); err != nil {
		return "", err
	}

	for _, file := range cu.RequiredInput {
		b, err := fetcher.Fetch(file.Info.Path, file.Info.Digest)
		if err != nil {
			return "", fmt.Errorf("fetching file %s: %v", file.Info.Path, err)
		}
		if _, err := w.AddFile(bytes.NewReader(b)); err != nil {
			return "", fmt.Errorf("writing file to kzip %s: %v", file.Info.Path, err)
		}
	}

	return tmpfile.Name(), nil
}
