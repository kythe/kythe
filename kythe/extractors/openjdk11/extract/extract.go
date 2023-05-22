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
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"regexp"
	"strings"

	"kythe.io/kythe/go/util/flagutil"
	"kythe.io/kythe/go/util/log"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
)

const (
	javaCommandVar      = "KYTHE_JAVA_COMMAND"
	wrapperExtractorVar = "KYTHE_JAVA_EXTRACTOR_JAR"

	kytheRootVar    = "KYTHE_ROOT_DIRECTORY"
	kytheOutputVar  = "KYTHE_OUTPUT_DIRECTORY"
	kytheCorpusVar  = "KYTHE_CORPUS"
	kytheVNameVar   = "KYTHE_VNAMES"
	kytheExcludeVar = "KYTHE_OPENJDK11_EXCLUDE_MODULES"

	javaMakeVar           = "JAVA_CMD"
	runfilesPrefix        = "${RUNFILES}"
	runfilesWrapperPath   = "kythe/extractors/openjdk11/java_wrapper/java_wrapper"
	runfilesVNamesPath    = "kythe/extractors/openjdk11/vnames.json"
	runfilesExtractorPath = "kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac_extractor_deploy.jar"
)

var (
	sourceDir      string
	buildDir       string
	makeTargets    = targetList{"clean", "jdk"}
	outputDir      string
	excludeModules flagutil.StringSet
	vNameRules     = defaultVNamesPath()
	wrapperPath    = defaultWrapperPath()
	extractorPath  = defaultExtractorPath()
	errorPattern   = regexp.MustCompile("ERROR: extractor failure for module ([^:]*):")
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

// Get implements part of the flag.Getter interface for targetList.
func (tl *targetList) Get() interface{} {
	return []string(*tl)
}

// runfilePath is a simple flag wrapping a possibly runfiles-relative path.
type runfilePath struct{ value string }

// Set implements part of the flag.Getter interface for targetList and will
// set the new value from s.
func (rp *runfilePath) Set(s string) error {
	rp.value = s
	return nil
}

// String implements part of the flag.Getter interface for runfilePath
// and will return the value.
func (rp *runfilePath) String() string {
	return rp.value
}

// Get implements part of the flag.Getter interface for runfilePath
// and will return the value after replacing a ${RUNFILES} prefix.
func (rp *runfilePath) Get() interface{} {
	return rp.String()
}

// Expand returns the expanded runfile path from its argument.
func (rp *runfilePath) Expand() string {
	if path := strings.TrimPrefix(rp.value, "${RUNFILES}"); path != rp.value {
		path, err := bazel.Runfile(path)
		if err != nil {
			panic(err)
		}
		return path
	}
	return rp.value
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

func defaultWrapperPath() runfilePath {
	return runfilePath{filepath.Join(runfilesPrefix, runfilesWrapperPath)}
}

func defaultVNamesPath() runfilePath {
	return runfilePath{filepath.Join(runfilesPrefix, runfilesVNamesPath)}
}

func defaultExtractorPath() runfilePath {
	return runfilePath{filepath.Join(runfilesPrefix, runfilesExtractorPath)}
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
	cmd.Dir = buildDir
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
	if path := vNameRules.Expand(); path != "" {
		env = setEnvDefault(env, kytheVNameVar, path)
	}
	if len(excludeModules) > 0 {
		env = append(env, kytheExcludeVar+"="+excludeModules.String())
	}
	env = append(env,
		kytheRootVar+"="+sourceDir,
		kytheOutputVar+"="+outputDir,
		wrapperExtractorVar+"="+extractorPath.Expand())
	return env
}

func init() {
	setupRunfiles()
	flag.StringVar(&sourceDir, "jdk", "", "path to the OpenJDK11 source tree (required)")
	flag.StringVar(&buildDir, "build", "", "path to the OpenJDK11 build tree (defaults to -jdk)")
	flag.StringVar(&outputDir, "output", defaultOutputDir(), "path to which the compilations and errors should be written (optional)")
	flag.Var(&vNameRules, "rules", "path of vnames.json file (optional)")
	flag.Var(&wrapperPath, "java_wrapper", "path to the java_wrapper executable (optional)")
	flag.Var(&extractorPath, "extractor_jar", "path to the javac_extractor_deploy.jar (optional)")
	flag.Var(&makeTargets, "targets", "comma-separated list of make targets to build")
	flag.Var(&excludeModules, "exclude_modules", "comma-separated set of module names to skip")
	flag.Usage = flagutil.SimpleUsage("Extract a configured openjdk11 source directory", "[--java_wrapper=] [path]")
}

func main() {
	flag.Parse()
	if sourceDir == "" {
		flagutil.UsageError("missing -jdk")
	}
	if wrapperPath.String() == "" {
		flagutil.UsageError("missing -java_wrapper")
	}
	if _, err := os.Stat(wrapperPath.Expand()); err != nil {
		flagutil.UsageErrorf("java_wrapper not found: %v", err)
	}
	if buildDir == "" {
		buildDir = sourceDir
	}

	cmd := exec.Command("make", append([]string{javaMakeVar + "=" + wrapperPath.Expand(), "ENABLE_JAVAC_SERVER=no"}, makeTargets...)...)
	cmd.Dir = buildDir
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
		log.Error(err)
	}
	if len(errors) > 0 {
		log.Error("Error extracting modules:\n\t", strings.Join(errors, "\n\t"))
	}
}
