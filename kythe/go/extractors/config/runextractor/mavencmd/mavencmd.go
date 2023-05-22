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

// Package mavencmd extracts a maven repo.
package mavencmd // import "kythe.io/kythe/go/extractors/config/runextractor/mavencmd"

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"strings"

	"kythe.io/kythe/go/extractors/config/preprocessor/modifier"
	"kythe.io/kythe/go/extractors/config/runextractor/backup"
	"kythe.io/kythe/go/extractors/constants"
	"kythe.io/kythe/go/util/cmdutil"
	"kythe.io/kythe/go/util/log"

	"github.com/google/subcommands"
)

type mavenCommand struct {
	cmdutil.Info

	pomXML       string
	javacWrapper string
	verbose      bool
}

// New creates a new subcommand for running maven extraction.
func New() subcommands.Command {
	return &mavenCommand{
		Info: cmdutil.NewInfo("maven", "extract a repo built with maven",
			`docstring TBD`),
	}
}

// SetFlags implements the subcommands interface and provides command-specific
// flags for maven extraction.
func (m *mavenCommand) SetFlags(fs *flag.FlagSet) {
	fs.StringVar(&m.javacWrapper, "javac_wrapper", "", "A required executable that wraps javac for Kythe extraction.")
	fs.StringVar(&m.pomXML, "pom_xml", "pom.xml", "The config file for a maven repo, defaults to 'pom.xml'")
	fs.BoolVar(&m.verbose, "v", false, "Enable verbose mode to print more debug info.")
}

func (m mavenCommand) checkFlags() error {
	for _, key := range constants.RequiredJavaEnv {
		if os.Getenv(key) == "" {
			return fmt.Errorf("required env var %s not set", key)
		}
	}
	if m.pomXML == "" {
		return fmt.Errorf("maven pom XML (e.g. 'pom.xml') not set")
	}
	if m.javacWrapper == "" {
		return fmt.Errorf("required -javac_wrapper not set")
	}
	return nil
}

// Execute implements the subcommands interface and runs maven extraction.
func (m *mavenCommand) Execute(ctx context.Context, fs *flag.FlagSet, args ...any) subcommands.ExitStatus {
	if err := m.checkFlags(); err != nil {
		return m.Fail("invalid flags: %v", err)
	}
	tf, err := backup.New(m.pomXML)
	if err != nil {
		return m.Fail("error backing up %s: %v", m.pomXML, err)
	}
	defer tf.Release()
	if err := modifier.PreProcessPomXML(m.pomXML); err != nil {
		return m.Fail("error modifying maven pom XML %s: %v", m.pomXML, err)
	}

	if m.verbose {
		// Print diff to show changes made to pom.xml.
		log.Infof("Modified pom.xml. Diff:")
		diff, err := tf.GetDiff()
		if err != nil {
			m.Fail("Error diffing pom.xml: %v", err)
		}
		log.Info(diff)
	}

	mvnArgs := []string{"clean", "install",
		"-Dmaven.compiler.forceJavaCompilerUser=true",
		"-Dmaven.compiler.fork=true",
		fmt.Sprintf("-Dmaven.compiler.executable=%s", m.javacWrapper),
	}
	log.Infof("Running `mvn %v`", strings.Join(mvnArgs, " "))
	cmd := exec.Command("mvn", mvnArgs...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return m.Fail("error executing maven build: %v", err)
	}
	// We restore the original config file even during successful runs because
	// the repo might persist outside of the calling docker image.  If that ends
	// up never happening in the future, could consider pulling this block
	// inside the above error case(s).
	if err := tf.Restore(); err != nil {
		return m.Fail("error restoring %s from %s: %v", m.pomXML, tf, err)
	}
	return subcommands.ExitSuccess
}
