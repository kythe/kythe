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

// Package mavencmd extracts a gradle repo.
package mavencmd

import (
	"context"
	"flag"
	"fmt"
	"os/exec"

	"kythe.io/kythe/go/extractors/config/runextractor/backup"
	"kythe.io/kythe/go/util/cmdutil"

	"github.com/google/subcommands"
)

type mavenCommand struct {
	cmdutil.Info

	buildFile       string
	javacWrapper    string
	pomPreProcessor string
}

func New() subcommands.Command {
	return &mavenCommand{
		Info: cmdutil.NewInfo("maven", "extract a repo built with maven",
			`docstring TBD`),
	}
}

func (m *mavenCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&m.javacWrapper, "javac_wrapper", "", "A required executable that wraps javac for Kythe extraction.")
	// TODO(#2905): Consider replacing this with a native go library, not executing out to a jar.
	fs.StringVar(&m.pomPreProcessor, "pom_pre_processor", "", "A required jar that preprocesses a maven file in preparation for Kythe Extraction.")
	fs.StringVar(&m.buildFile, "build_file", "pom.xml", "The config file for a maven repo, defaults to 'pom.xml'")
}

func (m mavenCommand) verifyFlags() error {
	if m.buildFile == "" {
		return fmt.Errorf("gradle build file (e.g. 'pom.xml') not set")
	}
	if m.javacWrapper == "" {
		return fmt.Errorf("required -javac_wrapper not set")
	}
	if m.pomPreProcessor == "" {
		return fmt.Errorf("required -pom_pre_processor not set")
	}
	return nil
}

func (m *mavenCommand) Execute(ctx context.Context, fs *flag.FlagSet, args ...interface{}) subcommands.ExitStatus {
	if err := m.verifyFlags(); err != nil {
		return m.Fail("invalid flags: %v", err)
	}
	tf, err := backup.Save(m.buildFile)
	if err != nil {
		return m.Fail("error backing up %s: %v", m.buildFile, err)
	}
	if err := exec.Command("java", "-jar", m.pomPreProcessor, "-pom", m.buildFile).Run(); err != nil {
		return m.Fail("error modifying maven build file %s: %v", m.buildFile, err)
	}
	if err := exec.Command("mvn", "clean", "install",
		"-Dmaven.compiler.forceJavaCompilerUser=true",
		"-Dmaven.compiler.fork=true",
		fmt.Sprintf("-Dmaven.compiler.executable=%s", m.javacWrapper)).Run(); err != nil {
		return m.Fail("error executing maven build: %v", err)
	}
	if err := backup.Restore(m.buildFile, tf); err != nil {
		return m.Fail("error restoring %s from %s: %v", m.buildFile, tf, err)
	}
	return subcommands.ExitSuccess
}
