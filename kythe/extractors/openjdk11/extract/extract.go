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
	"path/filepath"
	"regexp"
	"strings"

	"kythe.io/kythe/go/util/flagutil"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
)

const (
	javaCommandVar      = "KYTHE_JAVA_COMMAND"
	kytheRootVar        = "KYTHE_ROOT_DIRECTORY"
	kytheVnameVar       = "KYTHE_VNAMES"
	javaMakeVar         = "JAVA_CMD"
	runfilesWrapperPath = "io_kythe/kythe/extractors/openjdk11/java_wrapper"
	runfilesVnamesPath  = "io_kythe/kythe/extractors/openjdk11/vnames.json"
)

var (
	wrapperPath  string
	vnameRules   string
	errorPattern = regexp.MustCompile("ERROR: extractor failure for module ([^:]*):")
)

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

func defaultVnamesPath() string {
	val, _ := bazel.Runfile(runfilesVnamesPath)
	return val
}

func makeDir() string {
	if dir := flag.Arg(0); dir != "" {
		return dir
	}
	return "."
}

func findJavaCommand() (string, error) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	cmd := exec.CommandContext(ctx, "make", "-n", "-p")
	cmd.Dir = makeDir()
	cmd.Stderr = os.Stderr
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return "", err
	}
	if err := cmd.Start(); err != nil {
		return "", err
	}
	prefix := javaMakeVar + " := "
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
		log.Fatalf("unable to determine KYTHE_JAVA_COMMAND: %v", err)
	}
	return java
}

func setEnvDefaultFunc(env []string, key string, value func() string) []string {
	if val := os.Getenv(key); len(val) == 0 {
		env = append(env, key+"="+value())
	}
	return env
}

func makeEnv() []string {
	env := os.Environ()
	env = setEnvDefaultFunc(env, javaCommandVar, mustFindJavaCommand)
	env = setEnvDefaultFunc(env, kytheRootVar, makeDir)
	if len(vnameRules) > 0 {
		env = setEnvDefaultFunc(env, kytheVnameVar, func() string { return vnameRules })
	}
	return env
}

func init() {
	setupRunfiles()
	flag.StringVar(&wrapperPath, "java-wrapper", defaultWrapperPath(), "path to the java_wrapper executable (optional)")
	flag.StringVar(&vnameRules, "rules", defaultVnamesPath(), "path of vnames.json file (optional)")
	flag.Usage = flagutil.SimpleUsage("Extract a configured openjdk11 source directory", "[--java-wrapper=] [path]")
}

func main() {
	flag.Parse()

	if len(wrapperPath) == 0 {
		flagutil.UsageError("missing java-wrapper")
	}
	if _, err := os.Stat(wrapperPath); err != nil {
		flagutil.UsageErrorf("java-wrapper not found: %v", err)
	}

	cmd := exec.Command("make", javaMakeVar+"="+wrapperPath, "ENABLE_JAVAC_SERVER=no", "clean", "jdk")
	cmd.Dir = makeDir()
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
