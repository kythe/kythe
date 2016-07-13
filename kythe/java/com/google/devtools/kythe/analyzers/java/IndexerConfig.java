/*
 * Copyright 2015 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.analyzers.java;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.Parameters;

/** Common configuration for the Kythe Java indexer. */
@Parameters(separators = "=")
public class IndexerConfig {
  private final String programName;

  @Parameter(
    names = {"--help", "-h"},
    description = "Help requested",
    help = true
  )
  private boolean help;

  @Parameter(
    names = "--ignore_vname_paths",
    description =
        "Determines whether the analyzer should ignore the path components of the"
            + " {@link VName}s in each compilation.  This can be used to \"fix\" the coherence"
            + " of {@link VName}s across compilations when the extractor was not (or could not be)"
            + " supplied with a proper {@link VName}s configuration file.  Each path will instead be"
            + " set to the qualified name of each node's enclosing class (e.g. \"java.lang.String\""
            + " or \"com.google.common.base.Predicate\")."
  )
  private boolean ignoreVNamePaths;

  @Parameter(
    names = "--verbose",
    description =
        "Determines whether the analyzer should emit verbose logging messages for debugging."
  )
  private boolean verboseLogging;

  public IndexerConfig(String programName) {
    this.programName = programName;
  }

  /**
   * Parses the given command-line arguments, setting each known flag. If --help is requested, the
   * binary's usage message will be printed and the program will exit with non-zero code.
   */
  public final void parseCommandLine(String[] args) {
    JCommander jc = new JCommander(this, args);
    jc.setProgramName(programName);
    if (getHelp()) {
      jc.usage();
      System.exit(1);
    }
  }

  public final boolean getHelp() {
    return help;
  }

  public final boolean getIgnoreVNamePaths() {
    return ignoreVNamePaths;
  }

  public final boolean getVerboseLogging() {
    return verboseLogging;
  }

  public IndexerConfig setIgnoreVNamePaths(boolean ignoreVNamePaths) {
    this.ignoreVNamePaths = ignoreVNamePaths;
    return this;
  }

  public IndexerConfig setVerboseLogging(boolean verboseLogging) {
    this.verboseLogging = verboseLogging;
    return this;
  }
}
