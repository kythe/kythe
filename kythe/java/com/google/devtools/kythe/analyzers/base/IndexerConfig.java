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

package com.google.devtools.kythe.analyzers.base;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.Parameters;

/** Common configuration for Kythe indexers. */
@Parameters(separators = "=")
public class IndexerConfig {
  @Parameter(
    names = {"--help", "-h"},
    description = "Help requested",
    help = true
  )
  private boolean help;

  @Parameter(
    names = "--verbose",
    description =
        "Determines whether the analyzer should emit verbose logging messages for debugging."
  )
  private boolean verboseLogging;

  @Parameter(
    names = "--default_metadata_corpus",
    description =
        "If set, use this as the corpus for VNames generated from metadata (if a corpus cannot otherwise be determined)."
  )
  private String defaultMetadataCorpus;

  private final JCommander jc;

  public IndexerConfig(String programName) {
    jc = new JCommander(this);
    jc.setProgramName(programName);
  }

  /**
   * Parses the given command-line arguments, setting each known flag. If --help is requested, the
   * binary's usage message will be printed and the program will exit with non-zero code.
   */
  public final void parseCommandLine(String[] args) {
    jc.parse(args);
    if (getHelp()) {
      showHelpAndExit();
    }
  }

  /** Print the binary's usage message, and exit with a non-zero code. */
  public void showHelpAndExit() {
    jc.usage();
    System.exit(1);
  }

  public final boolean getHelp() {
    return help;
  }

  public final boolean getVerboseLogging() {
    return verboseLogging;
  }

  public final String getDefaultMetadataCorpus() {
    return defaultMetadataCorpus;
  }

  public IndexerConfig setVerboseLogging(boolean verboseLogging) {
    this.verboseLogging = verboseLogging;
    return this;
  }

  public IndexerConfig setDefaultMetadataCorpus(String defaultMetadataCorpus) {
    this.defaultMetadataCorpus = defaultMetadataCorpus;
    return this;
  }
}
