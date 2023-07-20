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

// Binary java_wrapper wraps the real java command to invoke the standalone java extractor
// in parallel with the genuine compilation command.
// As it interjects itself between make and a real java command, all options are
// provided via environment variables.  See the corresponding consts and java_extractor for details.
package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"

	"kythe.io/kythe/go/util/log"

	"bitbucket.org/creachadair/shell"
	"bitbucket.org/creachadair/stringset"
	"golang.org/x/sys/unix"
)

const (
	javaCommandVar    = "KYTHE_JAVA_COMMAND"              // Path to the real java command, required.
	extractorJarVar   = "KYTHE_JAVA_EXTRACTOR_JAR"        // Path to the javac_extractor jar, required.
	kytheOutputVar    = "KYTHE_OUTPUT_DIRECTORY"          // Extraction output directory, required.
	kytheTargetVar    = "KYTHE_ANALYSIS_TARGET"           // Extraction analysis target to set, defaults to the java module name.
	excludeModulesVar = "KYTHE_OPENJDK11_EXCLUDE_MODULES" // Names of for which to skip extraction.
)

var (
	modulePattern = regexp.MustCompile("^@/.*/_the.(.*)_batch.tmp$")
)

func moduleName() string {
	path := os.Args[len(os.Args)-1]
	// Hackish way to determine the likely module being compiled.
	repl := modulePattern.ReplaceAllString(path, "$1")
	if repl == path { // No match, use dirname and basename to form the module.
		return filepath.Base(filepath.Dir(path)) + "#" + filepath.Base(path)
	}
	return repl
}

func outputDir() string {
	if val := os.Getenv(kytheOutputVar); val != "" {
		return val
	}
	log.Fatal("ERROR: KYTHE_OUTPUT_DIRECTORY not set")
	return ""
}

func mustGetEnvPath(key string) string {
	if val := os.Getenv(key); val != "" {
		if _, err := os.Stat(val); err != nil {
			log.Fatalf("invalid %s: %v", key, err)
		}
		return val
	}
	log.Fatal(key + " not set")
	return ""
}

func loadExclusions() stringset.Set {
	if value := os.Getenv(excludeModulesVar); value != "" {
		return stringset.FromKeys(strings.Split(value, ","))
	}
	return stringset.Set{}
}

func javaCommand() string {
	return mustGetEnvPath(javaCommandVar)
}

func extractorJar() string {
	return mustGetEnvPath(extractorJarVar)
}

func extractorArgs(args []string, jar string) []string {
	isJavac := false
	var result []string
	for len(args) > 0 {
		var a string
		var v string
		switch a, args = shift(args); a {
		case "-m", "--module":
			v, args = shift(args)
			if !strings.HasSuffix(v, ".javac.Main") {
				isJavac = false
				break
			}
			isJavac = true
			result = append(result,
				"--add-modules=java.logging,java.sql",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.main=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.util=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.file=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.api=ALL-UNNAMED",
				"--add-exports=jdk.compiler.interim/com.sun.tools.javac.code=ALL-UNNAMED",
				"-jar", jar, "-Xprefer:source")
		case "--doclint-format":
			_, args = shift(args)
		case "-Werror":
		default:
			switch {
			case strings.HasPrefix(a, "-Xplugin:depend"), strings.HasPrefix(a, "-Xlint:"), strings.HasPrefix(a, "-Xdoclint"):
			case strings.HasPrefix(a, "-Xmx"):
				result = append(result, "-Xmx3G")
			case !isJavac && strings.HasPrefix(a, "--") && len(args) > 0 && !strings.HasPrefix(args[0], "-"):
				// Add an = separator between "--arg" and its value for JVM arguments
				// in order to be friendlier to Bazel Java "launchers".
				// The JVM generally accepts either form, but the generic argument processing
				// in many launchers requires the "=" separator.
				v, args = shift(args)
				result = append(result, a+"="+v)
			default:
				result = append(result, a)
			}
		}
	}
	// As we can only do anything meaningful with Java compilations,
	// but wrap the java binary, don't attempt to extract other invocations.
	if !isJavac {
		return nil
	}
	return result
}

func extractorEnv() []string {
	env := os.Environ()
	env = setEnvDefault(env, kytheTargetVar, moduleName())
	return env
}

func setEnvDefault(env []string, key, def string) []string {
	if val := os.Getenv(key); val == "" {
		env = append(env, key+"="+def)
	}
	return env
}

func shift(args []string) (string, []string) {
	if len(args) > 0 {
		return args[0], args[1:]
	}
	return "", nil
}

func main() {
	excludeModules := loadExclusions()
	java := javaCommand()
	jar := extractorJar()
	if args := extractorArgs(os.Args[1:], jar); len(args) > 0 {
		if excludeModules.Contains(moduleName()) {
			log.Infof("*** Skipping: %s", moduleName())
		} else {
			cmd := exec.Command(java, args...)
			cmd.Env = extractorEnv()
			log.Infof("*** Extracting: %s", moduleName())
			if output, err := cmd.CombinedOutput(); err != nil {
				w, err := os.Create(filepath.Join(outputDir(), moduleName()+".err"))
				if err != nil {
					log.Fatalf("Error creating error log for module %s: %v", moduleName(), err)
				}
				fmt.Fprintf(w, "--- %s\n", shell.Join(args))
				w.Write(output)
				w.Close()

				// Log, but don't abort, on extraction failures.
				log.Errorf("extractor failure for module %s: %v", moduleName(), err)
			}
		}
	}
	// Always end by running the java command directly, as "java".
	os.Args[0] = "java"
	log.Fatal(unix.Exec(java, os.Args, os.Environ())) // If exec returns at all, it's an error.
}
