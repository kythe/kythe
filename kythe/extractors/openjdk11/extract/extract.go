/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

// Binary extract extracts a pre-configured OpenJDK11 source tree to produce Kythe compilation units.
package main

import (
	"bufio"
	"context"
	"errors"
	"flag"
	"io"
	"log"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"regexp"
	"strings"

	"kythe.io/kythe/go/util/flagutil"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
)

const (
	javaCommandVar      = "KYTHE_JAVA_COMMAND"
	wrapperExtractorVar = "KYTHE_JAVA_EXTRACTOR_JAR"

	kytheRootVar   = "KYTHE_ROOT_DIRECTORY"
	kytheOutputVar = "KYTHE_OUTPUT_DIRECTORY"
	kytheCorpusVar = "KYTHE_CORPUS"
	kytheVNameVar  = "KYTHE_VNAMES"

	javaMakeVar           = "JAVA_CMD"
	runfilesWrapperPath   = "io_kythe/kythe/extractors/openjdk11/java_wrapper"
	runfilesVNamesPath    = "io_kythe/kythe/extractors/openjdk11/vnames.json"
	runfilesExtractorPath = "io_kythe/kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac9_extractor_deploy.jar"
)

var (
	makeDir       string
	makeTargets   = targetList{"clean", "jdk"}
	outputDir     string
	vNameRules    string
	wrapperPath   string
	extractorPath string
	errorPattern  = regexp.MustCompile("ERROR: extractor failure for module ([^:]*):")
)

// targetList is a simple comma-separated list of strings used
// for the -targets flag.
// flagutil.StringList always accumulates values, which we don't want.
type targetList []string

// Set implements part of the flag.Getter interface for targetList and will
// set the new value from s.
func (tl *targetList) Set(s string) error {
	*tl = strings.Split(s, ",")
	return nil
}

// String implements part of the flag.Getter interface for targetList and will
// return the value as a comma-separate string.
func (tl *targetList) String() string {
	if tl == nil {
		return ""
	}
	return strings.Join(*tl, ",")
}

func setupRunfiles() error {
	if os.Getenv("RUNFILES_DIR") != "" || os.Getenv("RUNFILES_MANIFEST_FILE") != "" {
		return nil
	}
	for _, base := range []string{os.Args[0] + ".runfiles", "."} {
		root, err := filepath.Abs(base)
		if err != nil {
			continue
		} else if _, err := os.Stat(root); err != nil {
			continue
		}
		os.Setenv("RUNFILES_DIR", root)
		manifest := filepath.Join(root, "MANIFEST")
		if _, err := os.Stat(manifest); err == nil {
			os.Setenv("RUNFILES_MANIFEST_FILE", manifest)
		}
		return nil
	}
	return errors.New("unable to setup runfiles")
}

func defaultWrapperPath() string {
	val, _ := bazel.Runfile(runfilesWrapperPath)
	return val
}

func defaultVNamesPath() string {
	val, _ := bazel.Runfile(runfilesVNamesPath)
	return val
}

func defaultExtractorPath() string {
	val, _ := bazel.Runfile(runfilesExtractorPath)
	return val
}

func defaultOutputDir() string {
	val := os.Getenv(kytheOutputVar)
	if val == "" {
		usr, err := user.Current()
		if err != nil {
			log.Fatalf("ERROR: unable to determine current user: %v", err)
		}
		val = filepath.Join(usr.HomeDir, "kythe-openjdk11-output")
	}
	return val
}

func findJavaCommand() (string, error) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	cmd := exec.CommandContext(ctx, "make", "-n", "-p")
	cmd.Dir = makeDir
	cmd.Stderr = os.Stderr
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return "", err
	}
	if err := cmd.Start(); err != nil {
		return "", err
	}
	const prefix = javaMakeVar + " := "
	scanner := bufio.NewScanner(stdout)
	for scanner.Scan() {
		if strings.HasPrefix(scanner.Text(), prefix) {
			cancel() // Safe to call repeatedly.
			cmd.Wait()
			return strings.TrimPrefix(scanner.Text(), prefix), nil
		}
	}
	return "", cmd.Wait()
}

func mustFindJavaCommand() string {
	java, err := findJavaCommand()
	if err != nil {
		log.Fatalf("unable to determine %s: %v", javaCommandVar, err)
	}
	return java
}

func setEnvDefaultFunc(env []string, key string, value func() string) []string {
	if val := os.Getenv(key); val == "" {
		env = append(env, key+"="+value())
	}
	return env
}

func setEnvDefault(env []string, key, value string) []string {
	return setEnvDefaultFunc(env, key, func() string { return value })
}

func makeEnv() []string {
	env := os.Environ()
	env = setEnvDefaultFunc(env, javaCommandVar, mustFindJavaCommand)
	env = setEnvDefault(env, kytheCorpusVar, "openjdk11")
	if vNameRules != "" {
		env = setEnvDefault(env, kytheVNameVar, vNameRules)
	}
	env = append(env,
		kytheRootVar+"="+makeDir,
		kytheOutputVar+"="+outputDir,
		wrapperExtractorVar+"="+extractorPath)
	return env
}

func init() {
	setupRunfiles()
	flag.StringVar(&makeDir, "jdk", "", "path to the OpenJDK11 source tree (required)")
	flag.StringVar(&outputDir, "output", defaultOutputDir(), "path to which the compilations and errors should be written (optional)")
	flag.StringVar(&vNameRules, "rules", defaultVNamesPath(), "path of vnames.json file (optional)")
	flag.StringVar(&wrapperPath, "java_wrapper", defaultWrapperPath(), "path to the java_wrapper executable (optional)")
	flag.StringVar(&extractorPath, "extractor_jar", defaultExtractorPath(), "path to the javac_extractor_deploy.jar (optional)")
	flag.Var(&makeTargets, "targets", "comma-separated list of make targets to build")
	flag.Usage = flagutil.SimpleUsage("Extract a configured openjdk11 source directory", "[--java_wrapper=] [path]")
}

func main() {
	flag.Parse()
	if makeDir == "" {
		flagutil.UsageError("missing -jdk")
	}
	if wrapperPath == "" {
		flagutil.UsageError("missing -java_wrapper")
	}
	if _, err := os.Stat(wrapperPath); err != nil {
		flagutil.UsageErrorf("java_wrapper not found: %v", err)
	}

	cmd := exec.Command("make", append([]string{javaMakeVar + "=" + wrapperPath, "ENABLE_JAVAC_SERVER=no"}, makeTargets...)...)
	cmd.Dir = makeDir
	cmd.Env = makeEnv()
	cmd.Stdout = nil // Quiet, you
	stderr, err := cmd.StderrPipe()

	if err != nil {
		log.Fatal(err)
	}

	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}

	logCollectedErrors(io.TeeReader(stderr, os.Stderr))

	if err := cmd.Wait(); err != nil {
		log.Fatal(err)
	}
}

func collectExtractionErrors(stderr io.Reader) ([]string, error) {
	var result []string
	scanner := bufio.NewScanner(stderr)
	for scanner.Scan() {
		if subs := errorPattern.FindSubmatch(scanner.Bytes()); subs != nil {
			result = append(result, string(subs[1]))
		}
	}
	return result, scanner.Err()
}

func logCollectedErrors(stderr io.Reader) {
	errors, err := collectExtractionErrors(stderr)
	if err != nil {
		log.Println(err)
	}
	log.Println("Error extracting modules:\n\t", strings.Join(errors, "\n\t"))
}
