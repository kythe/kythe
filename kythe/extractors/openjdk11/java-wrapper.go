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

// java-wrapper wraps the real java command to invoke the standalond java extractor
// in parallel with the genuine compilation command.
package main

import (
	"errors"
	"fmt"
	"log"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"regexp"
	"strings"

	"bitbucket.org/creachadair/shell"
	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"golang.org/x/sys/unix"
)

const (
	javaCommandVar    = "KYTHE_JAVA_COMMAND"
	javaHomeVar       = "JAVA_HOME"
	jdkVersionVar     = "JDK_VERSION"
	extractorJarVar   = "JAVAC_EXTRACTOR_JAR"
	kytheRootVar      = "KYTHE_ROOT_DIRECTORY"
	kytheOutputVar    = "KYTHE_OUTPUT_DIRECTORY"
	kytheCorpusVar    = "KYTHE_CORPUS"
	kytheTargetVar    = "KYTHE_ANALYSIS_TARGET"
	defaultJdkVersion = "11"
	extractorJarPath  = "io_kythe/kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac9_extractor_deploy.jar"
)

var (
	modulePattern = regexp.MustCompile("^@/.*/_the.(.*)_batch.tmp$")
)

func moduleName() string {
	// Hackish way to determine the likely module being compiled.
	return modulePattern.ReplaceAllString(os.Args[len(os.Args)-1], "$1")
}

func outputDir() string {
	val := os.Getenv(kytheOutputVar)
	if len(val) == 0 {
		usr, err := user.Current()
		if err != nil {
			log.Fatalf("ERROR: unable to determine current user: %v", err)
		}
		val = filepath.Join(usr.HomeDir, "kythe-openjdk11-output")
	}
	return val
}

func targetJdk() string {
	if val := os.Getenv(jdkVersionVar); len(val) > 0 {
		return val
	}
	return defaultJdkVersion
}

func checkPath(parts ...string) error {
	_, err := os.Stat(filepath.Join(parts...))
	return err
}

func setupRunfiles() error {
	if len(os.Getenv("RUNFILES_DIR")) > 0 || len(os.Getenv("RUNFILES_MANIFEST_FILE")) > 0 {
		return nil
	}
	for _, base := range []string{os.Args[0] + ".runfiles", "."} {
		if root, err := filepath.Abs(base); err == nil {
			if _, err := os.Stat(root); err == nil {
				os.Setenv("RUNFILES_DIR", root)
				if _, err := os.Stat(filepath.Join(root, "MANIFEST")); err == nil {
					os.Setenv("RUNFILES_MANIFEST_FILE", filepath.Join(root, "MANIFEST"))
				}
				return nil
			}
		}
	}
	return errors.New("unable to setup runfiles")
}

func findJavaCommand() (string, error) {
	if val := os.Getenv(javaCommandVar); len(val) > 0 {
		return val, checkPath(val)
	}
	if val := os.Getenv(javaHomeVar); len(val) > 0 {
		return filepath.Join(val, "bin", "java"), checkPath(val, "bin", "java")
	}
	return exec.LookPath("java")
}

func findExtractorJar() (string, error) {
	if val := os.Getenv(extractorJarVar); len(val) > 0 {
		return val, checkPath(val)
	}
	return bazel.Runfile(extractorJarPath)
}

func extractorArgs(args []string, jar string) []string {
	target := targetJdk()
	isJavac := false
	isBootstrap := true
	var result []string
	for len(args) > 0 {
		switch a := shift(&args); a {
		case "-m", "--module":
			v := shift(&args)
			if !strings.HasSuffix(v, ".javac.Main") {
				// This is not a javac invocation, don't extract.
				return nil
			}
			isJavac = true
			result = append(result,
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.main=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.util=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.file=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.api=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.code=ALL-UNNAMED",
				"-jar", jar, "-Xprefer:source")
		case "-target":
			v := shift(&args)
			if v != target {
				// This is definitely a bootstrap compilation, don't extract it.
				return nil
			}
			isBootstrap = false
			result = append(result, a, v)
		case "--add-modules", "--limit-modules":
			result = append(result, a, shift(&args)+",java.logging,java.sql")
		case "--doclint-format":
			shift(&args)
		case "-Werror":
		default:
			switch {
			case strings.HasPrefix(a, "-Xplugin:depend"), strings.HasPrefix(a, "-Xlint:"), strings.HasPrefix(a, "-Xdoclint"):
			case strings.HasPrefix(a, "-Xmx"):
				result = append(result, "-Xmx3G")
			default:
				result = append(result, a)
			}
		}
	}
	if !isJavac || isBootstrap {
		return nil
	}
	return result
}

func extractorEnv() []string {
	env := os.Environ()
	env = setenvDefault(env, kytheRootVar, ".")
	env = setenvDefault(env, kytheOutputVar, outputDir())
	env = setenvDefault(env, kytheTargetVar, moduleName())
	env = setenvDefault(env, kytheCorpusVar, "openjdk"+targetJdk())
	return env
}

func setenvDefault(env []string, key, def string) []string {
	if val := os.Getenv(key); len(val) == 0 {
		env = append(env, key+"="+def)
	}
	return env
}

func shift(args *[]string) string {
	if len(*args) > 0 {
		head := (*args)[0]
		*args = (*args)[1:]
		return head
	}
	return ""
}

func init() {
	setupRunfiles()
}

func main() {
	java, err := findJavaCommand()
	if err != nil {
		log.Fatalf("ERROR: unable to find java executable: %v", err)
	}
	jar, err := findExtractorJar()
	if err != nil {
		log.Fatalf("ERROR: unable to find extractor jar: %v", err)
	}
	if args := extractorArgs(os.Args[1:], jar); len(args) > 0 {
		cmd := exec.Command(java, args...)
		cmd.Env = extractorEnv()
		log.Printf("*** Extracting: %s", moduleName())
		if output, err := cmd.CombinedOutput(); err != nil {
			w, err := os.Create(filepath.Join(outputDir(), moduleName()+".err"))
			if err != nil {
				log.Printf("Error creating error log for module %s: %v", moduleName(), err)
			}
			defer w.Close()
			if _, err := fmt.Fprintf(w, "--- %s\n", shell.Join(args)); err != nil {
				log.Printf("Error writing error log for module %s: %v", moduleName(), err)
			}
			if _, err := w.Write(output); err != nil {
				log.Printf("Error writing error log for module %s: %v", moduleName(), err)
			}
			// Log, but don't abort, on extraction failures.
			log.Printf("ERROR: extractor failure for module %s: %v", moduleName(), err)
		}
	}
	// Always end by running the java command directly, as "java".
	os.Args[0] = "java"
	err = unix.Exec(java, os.Args, os.Environ())
	log.Fatal(err) // If exec returns at all, it's an error.
}
