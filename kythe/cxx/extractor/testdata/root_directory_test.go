package root_directory_test

import (
	"fmt"
	"log"
	"testing"

	"os"
	"os/exec"
	"path"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"github.com/golang/protobuf/proto"
	"github.com/golang/protobuf/ptypes"
	"github.com/golang/protobuf/ptypes/any"
	"github.com/google/go-cmp/cmp"

	"kythe.io/kythe/go/platform/kzip"

	apb "kythe.io/kythe/proto/analysis_go_proto"
	fcpb "kythe.io/kythe/proto/filecontext_go_proto"
	spb "kythe.io/kythe/proto/storage_go_proto"
)

var (
	testName         = "root_directory_test"
	extractorRunpath = "kythe/cxx/extractor/cxx_extractor"
	fileRunpath      = "kythe/cxx/extractor/testdata/altroot/altpath/file.cc"
	outputPath       = path.Join(os.Getenv("TEST_TMPDIR"), testName+".kzip")
	expectedUnit     = apb.CompilationUnit{
		VName: &spb.VName{
			Language: "c++",
		},
		RequiredInput: []*apb.CompilationUnit_FileInput{
			{
				VName: &spb.VName{Path: "altpath/file.cc"},
				Info: &apb.FileInfo{
					Path:   "file.cc",
					Digest: "a24091884bc15b53e380fe5b874d1bb52d89269fdf2592808ac70ba189204730",
				},
				Details: []*any.Any{
					mustMarshal(&fcpb.ContextDependentVersion{
						Row: []*fcpb.ContextDependentVersion_Row{
							{SourceContext: "hash0"},
						},
					}),
				},
			},
		},
		Argument: []string{
			"/dummy/bin/g++",
			"-target",
			"dummy-target",
			"-DKYTHE_IS_RUNNING=1",
			"-resource-dir",
			"/kythe_builtins",
			"--driver-mode=g++",
			"file.cc",
			"-fsyntax-only",
		},
		SourceFile:   []string{"file.cc"},
		EntryContext: "hash0",
	}
)

func TestRootDirectory(t *testing.T) {
	defer os.RemoveAll(outputPath)
	extractor, err := bazel.Runfile(extractorRunpath)
	if err != nil {
		t.Fatal(err)
	}
	filepath, err := bazel.Runfile(fileRunpath)
	if err != nil {
		t.Fatal(err)
	}
	cmd := exec.Command(extractor, "--with_executable", "/dummy/bin/g++", path.Base(filepath))
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Dir = path.Dir(filepath)
	cmd.Env = append(os.Environ(),
		"KYTHE_EXCLUDE_EMPTY_DIRS=1",
		"KYTHE_EXCLUDE_AUTOCONFIGURATION_FILES=1",
		"KYTHE_ROOT_DIRECTORY="+path.Dir(path.Dir(filepath)),
		"KYTHE_OUTPUT_FILE="+outputPath)
	if err := cmd.Run(); err != nil {
		t.Fatal(err)
	}
	units, err := readAll(outputPath)
	if err != nil {
		t.Fatal(err)
	}
	if len(units) != 1 {
		t.Fatalf("unexpected number of compilation units: got %d, want %d", len(units), 1)
	}

	// Remove or normalize fields that are likely to differ between runs.
	units[0].Proto.VName.Signature = ""
	units[0].Proto.WorkingDirectory = path.Clean(units[0].Proto.WorkingDirectory)
	units[0].Proto.Argument[2] = "dummy-target"
	units[0].Proto.Details = nil

	if err := canonicalizeContextHashes(units[0].Proto); err != nil {
		t.Fatal(err)
	}

	expectedUnit.WorkingDirectory = path.Clean(cmd.Dir)
	if diff := cmp.Diff(expectedUnit, *units[0].Proto); diff != "" {
		t.Fatalf("unexpected VName differences: (- expected; + found)\n%s", diff)
	}
}

func mustMarshal(msg proto.Message) *any.Any {
	any, err := ptypes.MarshalAny(msg)
	if err != nil {
		log.Fatal(err)
	}
	return any
}

func readAll(path string) ([]*kzip.Unit, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	var result []*kzip.Unit
	return result, kzip.Scan(file, func(_ *kzip.Reader, unit *kzip.Unit) error {
		result = append(result, unit)
		return nil
	})
}

func canonicalizeHash(hashes map[string]string, hash string) string {
	if hash == "" {
		return hash
	}
	size := len(hashes)
	found, ok := hashes[hash]
	if ok {
		return found
	}
	found = "hash" + fmt.Sprint(size)
	hashes[hash] = found
	return found
}

func canonicalizeContextHashes(unit *apb.CompilationUnit) error {
	hashes := make(map[string]string)
	unit.EntryContext = canonicalizeHash(hashes, unit.EntryContext)
	for _, input := range unit.RequiredInput {
		for i, any := range input.Details {
			var version fcpb.ContextDependentVersion
			if ptypes.Is(any, &version) {
				if err := ptypes.UnmarshalAny(any, &version); err != nil {
					return err
				}
			}
			for _, row := range version.Row {
				row.SourceContext = canonicalizeHash(hashes, row.SourceContext)
				for _, column := range row.Column {
					column.LinkedContext = canonicalizeHash(hashes, column.LinkedContext)
				}
			}
			any, err := ptypes.MarshalAny(&version)
			if err != nil {
				return err
			}
			input.Details[i] = any
		}
	}
	return nil
}
