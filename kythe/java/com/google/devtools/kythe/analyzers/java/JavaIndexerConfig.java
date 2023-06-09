/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

import com.beust.jcommander.Parameter;
import com.google.devtools.kythe.analyzers.base.IndexerConfig;

public class JavaIndexerConfig extends IndexerConfig {
  @Parameter(
      names = "--emit_doc_for_non_javadoc",
      description = "Emit documentation nodes for non-javadoc comments")
  private boolean emitDocForNonJavadoc;

  @Parameter(names = "--emit_jvm_signatures", description = "Generate vnames with jvm signatures.")
  private boolean emitJvmSignatures;

  @Parameter(
      names = "--ignore_vname_paths",
      description =
          "Determines whether the analyzer should ignore the path components of the {@link VName}s"
              + " in each compilation.  This can be used to \"fix\" the coherence of {@link"
              + " VName}s across compilations when the extractor was not (or could not be)"
              + " supplied with a proper {@link VName}s configuration file.  Each path will"
              + " instead be set to the qualified name of each node's enclosing class (e.g."
              + " \"java.lang.String\" or \"com.google.common.base.Predicate\").")
  private boolean ignoreVNamePaths;

  @Parameter(
      names = "--ignore_vname_roots",
      description =
          "Determines whether the analyzer should ignore the root components of the {@link VName}s"
              + " in each compilation.  This can be used to \"fix\" the coherence of {@link"
              + " VName}s across compilations when the extractor was not (or could not be)"
              + " supplied with a proper {@link VName}s configuration file.")
  private boolean ignoreVNameRoots;

  @Parameter(
      names = "--override_jdk_corpus",
      description =
          "If set, use this as the corpus for classes from java.*, javax.*, com.sun.*, and sun.*. "
              + " Anchor and file VNames are not affected.")
  private String overrideJdkCorpus;

  @Parameter(
      names = "--emit_anchor_scopes",
      description =
          "Whether to emit childof edges from anchors to their lexical scope's semantic node")
  private boolean emitAnchorScopes;

  @Parameter(
      names = "--temp_directory",
      description =
          "Directory on the local file system that can be used to store files that the java"
              + " compiler insists on being read from the local file system.")
  private String temporaryDirectory;

  @Parameter(
      names = "--use_compilation_corpus_as_default",
      description = "Use the CompilationUnit VName corpus as the default.")
  private boolean useCompilationCorpusAsDefault;

  @Parameter(
      names = "--emit_ref_call_over_identifier",
      description = "If true, emit ref/call anchor spans over the function identifier")
  private boolean emitRefCallOverIdentifier;

  public JavaIndexerConfig(String programName) {
    super(programName);
  }

  public final boolean getIgnoreVNamePaths() {
    return ignoreVNamePaths;
  }

  public final boolean getIgnoreVNameRoots() {
    return ignoreVNameRoots;
  }

  public final String getOverrideJdkCorpus() {
    return overrideJdkCorpus;
  }

  public boolean getEmitDocForNonJavadoc() {
    return emitDocForNonJavadoc;
  }

  public boolean getEmitJvmSignatures() {
    return emitJvmSignatures;
  }

  public boolean getEmitAnchorScopes() {
    return emitAnchorScopes;
  }

  public String getTemporaryDirectory() {
    return temporaryDirectory;
  }

  public boolean getUseCompilationCorpusAsDefault() {
    return useCompilationCorpusAsDefault;
  }

  public boolean getEmitRefCallOverIdentifier() {
    return emitRefCallOverIdentifier;
  }

  public JavaIndexerConfig setIgnoreVNamePaths(boolean ignoreVNamePaths) {
    this.ignoreVNamePaths = ignoreVNamePaths;
    return this;
  }

  public JavaIndexerConfig setIgnoreVNameRoots(boolean ignoreVNameRoots) {
    this.ignoreVNameRoots = ignoreVNameRoots;
    return this;
  }

  public JavaIndexerConfig setOverrideJdkCorpus(String overrideJdkCorpus) {
    this.overrideJdkCorpus = overrideJdkCorpus;
    return this;
  }

  public JavaIndexerConfig setEmitAnchorScopes(boolean emitAnchorScopes) {
    this.emitAnchorScopes = emitAnchorScopes;
    return this;
  }

  public JavaIndexerConfig setEmitDocForNonJavadoc(boolean emitDocForNonJavadoc) {
    this.emitDocForNonJavadoc = emitDocForNonJavadoc;
    return this;
  }

  public JavaIndexerConfig setEmitJvmSignatures(boolean emitJvmSignatures) {
    this.emitJvmSignatures = emitJvmSignatures;
    return this;
  }

  public JavaIndexerConfig setTemporaryDirectory(String temporaryDirectory) {
    this.temporaryDirectory = temporaryDirectory;
    return this;
  }

  public JavaIndexerConfig setUseCompilationCorpusAsDefault(boolean useCompilationCorpusAsDefault) {
    this.useCompilationCorpusAsDefault = useCompilationCorpusAsDefault;
    return this;
  }

  public JavaIndexerConfig setRefCallOverIdentifier(boolean emitRefCallOverIdentifier) {
    this.emitRefCallOverIdentifier = emitRefCallOverIdentifier;
    return this;
  }
}
