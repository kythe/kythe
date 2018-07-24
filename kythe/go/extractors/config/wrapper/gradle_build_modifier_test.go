package wrapper

import (
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"testing"

	"github.com/google/go-cmp/cmp"
)

const testDataDir = "testdata"

var multipleNewLines = regexp.MustCompile("\n{2,}")

// filesEqual compares two strings by collapsing irrelevant whitespace.
// It returns both a boolean indicating equality, as well as any relevant
// diff.
// TODO(#2860) combine this with imagesEqual in a util.
func filesEqual(got, want []byte) (bool, string) {
	// remove superfluous whitespace
	gotStr := strings.Trim(string(got[:]), " \n")
	wantStr := strings.Trim(string(want[:]), " \n")
	gotStr = multipleNewLines.ReplaceAllString(gotStr, "\n")
	wantStr = multipleNewLines.ReplaceAllString(wantStr, "\n")

	// diff want vs got
	diff := cmp.Diff(gotStr, wantStr)
	if diff != "" {
		return false, diff
	}

	return true, ""
}

func TestHasKythe(t *testing.T) {
	testcases := []struct {
		fileName string
		// Whether kythe javac wrapper should be present.
		hasWrapper bool
		// Whether we expect an error.
		wantError bool
	}{
		{fileName: "modified-gradle.build", hasWrapper: true},
		{fileName: "plain-gradle.build"},
		{fileName: "other-gradle.build", wantError: true},
	}

	for _, tcase := range testcases {
		// This should just ignore the file and do nothing.
		hasWrapper, err := hasKytheWrapper(getPath(tcase.fileName))
		if err != nil && !tcase.wantError {
			t.Fatalf("Failed to open test gradle file: %v", err)
		} else if err == nil && tcase.wantError {
			t.Errorf("Expected a failure for file %s, but didn't get one.", tcase.fileName)
		} else if tcase.hasWrapper != hasWrapper {
			var errstr string
			if tcase.hasWrapper {
				errstr = "should"
			} else {
				errstr = "should not"
			}
			t.Errorf("file %s %s already have kythe javac-wrapper", tcase.fileName, errstr)
		}
	}
}

func TestPreprocess(t *testing.T) {
	testcases := []struct {
		inputFile          string
		expectedOutputFile string
	}{
		{"modified-gradle.build", "modified-gradle.build"},
		{"plain-gradle.build", "modified-gradle.build"},
	}

	for _, tcase := range testcases {
		// Copy the file into a temp file.
		tf, err := ioutil.TempFile("", tcase.inputFile)
		if err != nil {
			t.Fatalf("creating temp file: %v", err)
		}
		tfName := tf.Name()
		defer os.Remove(tfName)
		infile, err := os.Open(getPath(tcase.inputFile))
		if err != nil {
			t.Fatalf("opening file %s: %v", tcase.inputFile, err)
		}
		_, err = io.Copy(tf, infile)
		if err != nil {
			t.Fatalf("copying %s: %v", tcase.inputFile, err)
		}
		if err := infile.Close(); err != nil {
			t.Fatalf("closing %s: %v", tcase.inputFile, err)
		}
		if err := tf.Close(); err != nil {
			t.Fatalf("closing temp file: %v", err)
		}

		// Do the copy if necessary.
		if err := PreProcessGradleBuild(tfName); err != nil {
			t.Fatalf("modifying gradle file %s: %v", tcase.inputFile, err)
		}

		// Compare results.
		eq, diff := filesEqual(readBytes(t, tfName), readBytes(t, getPath(tcase.expectedOutputFile)))
		if !eq {
			t.Errorf("Expected input file %s to be %s, but got diff %s", tcase.inputFile, tcase.expectedOutputFile, diff)
		}
	}
}

// getPath returns a resolved filepath name for a file living in testdata/ directory.
func getPath(f string) string {
	return os.ExpandEnv(filepath.Join(testDataDir, f))
}

func readBytes(t *testing.T, f string) []byte {
	b, err := ioutil.ReadFile(f)
	if err != nil {
		t.Fatalf("Failed to read %s: %v", f, err)
	}
	return b
}
