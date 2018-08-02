/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

// Binary runner provides a tool to wrap a Java compilation with Kythe's custom
// extractor logic.
//
// Before running this binary, make sure that any required environment variables
// for the underlying javac wrapper are set.  For example the default Kythe
// javac-wrapper.sh has requirements described at
// kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac-wrapper.sh
package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"os/exec"

	"kythe.io/kythe/go/extractors/config/wrapper"
)

var (
	javacWrapper = flag.String("javac_wrapper", "", "A required executable that wraps javac for Kythe extraction.")
	builder      = flag.String("builder", "", "A supported build system, either MAVEN or GRADLE.")

	// Flags specific for MAVEN build support
	mvnPomPreprocessor = flag.String("mvn_pom_preprocessor", "", "If builder is MAVEN, mvn_pom_preprocessor is a jar that makes necessary changes to pom.xml before extraction.")
	mavenBuildFile     = flag.String("mvn_build_file", "pom.xml", "The config file for a maven repo, defaults to 'pom.xml'")

	// Flags specific for GRADLE build support
	gradleBuildFile = flag.String("gradle_build_file", "gradle.build", "The config file for a gradle repo, defaults to 'gradle.build'")
)

type buildConfig struct {
	// The build file, for example pom.xml or gradle.build
	buildFile string
	// A function that modifies the given build file before extraction.
	preProcessor func(configFile string) error
}

func verifyFlags() (config buildConfig) {
	if flag.NArg() > 0 {
		log.Fatalf("Unknown arguments: %v", flag.Args())
	}

	hasError := false
	if *javacWrapper == "" {
		hasError = true
		log.Println("You must provide a -javac_wrapper")
	}

	switch *builder {
	// These are the supported builders.
	case "MAVEN":
		hasError = hasError || verifyMavenFlags()
		config.buildFile = *mavenBuildFile
		config.preProcessor = preProcessMavenBuild
	case "GRADLE":
		hasError = hasError || verifyGradleFlags()
		config.buildFile = *gradleBuildFile
		config.preProcessor = wrapper.PreProcessGradleBuild
	default:
		hasError = true
		log.Println("Unrecognized -builder, must be MAVEN or GRADLE")
	}

	if config.buildFile == "" {
		hasError = true
		log.Println("Failed to get valid build file, set either -mvn_build_file or -gradle_build_file accordingly")
	}
	if config.preProcessor == nil {
		config.preProcessor = func(configFile string) error {
			log.Println("No-op config preprocess for file ", configFile)
			return nil
		}
	}

	if hasError {
		os.Exit(1)
	}
	return
}

func verifyMavenFlags() (hasError bool) {
	if *mvnPomPreprocessor == "" {
		hasError = true
		log.Println("Must specify a valid -mvn_pom_preprocessor")
	}
	return
}

func verifyGradleFlags() bool {
	log.Println("GRADLE is still not supported")
	return true
}

func preProcessMavenBuild(configFile string) error {
	return exec.Command("java", "-jar", *mvnPomPreprocessor, "-pom", configFile).Run()
}

func copyBuildConfig(configFile string) (string, error) {
	// Copy over the build file temporarily so we can undo any hacks.
	bf, err := os.Open(configFile)
	if err != nil {
		return "", fmt.Errorf("opening config file %s", configFile)
	}
	defer bf.Close()
	tf, err := ioutil.TempFile("", "tmp-build-file")
	if err != nil {
		return "", fmt.Errorf("opening temp file")
	}
	defer tf.Close()
	_, err = io.Copy(tf, bf)
	return tf.Name(), err
}

func restoreBuildConfig(configFile string, tempFile string) {
	tf, err := os.Open(tempFile)
	if err != nil {
		log.Panicf("Failed to open temp file %s while copying back to original build file %s", tempFile, configFile)
	}
	defer tf.Close()
	bf, err := os.OpenFile(configFile, os.O_RDWR|os.O_CREATE, 0644)
	if err != nil {
		log.Panicf("Failed to open config file %s while copying back from temp file", configFile)
	}
	defer bf.Close()
	_, err = io.Copy(bf, tf)
	if err != nil {
		log.Panicf("Failed to copy from temp file %s to config file %s", tempFile, configFile)
	}
}

func runExtraction() error {
	// Do the actual extraction.
	switch *builder {
	case "MAVEN":
		return exec.Command(
			"mvn", "clean", "install",
			"-Dmaven.compiler.forceJavaCompilerUser=true",
			"-Dmaven.compiler.fork=true",
			fmt.Sprintf("-Dmaven.compiler.executable=%s", *javacWrapper)).Run()
	case "GRADLE":
		return exec.Command("gradle", "build").Run()
	}
	return fmt.Errorf("extraction failed, unrecognized builder %s", *builder)
}

func main() {
	flag.Parse()
	config := verifyFlags()

	temp, err := copyBuildConfig(config.buildFile)
	if err != nil {
		log.Panicf("Failed to copy build config to temp file: %v", err)
	}
	defer restoreBuildConfig(config.buildFile, temp)

	if err := config.preProcessor(config.buildFile); err != nil {
		log.Panicf("Failed to preprocess build file %s: %v", config.buildFile, err)
	}

	if err := runExtraction(); err != nil {
		log.Panicf("Failed to run the javac wrapper: %v", err)
	}
}
