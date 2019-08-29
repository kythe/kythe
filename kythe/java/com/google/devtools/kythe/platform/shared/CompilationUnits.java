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

package com.google.devtools.kythe.platform.shared;

import com.google.common.collect.Lists;
import com.google.common.hash.HashFunction;
import com.google.common.hash.Hashing;
import com.google.common.io.ByteArrayDataOutput;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.Env;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.protobuf.Any;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/** Utilities for working with CompilationUnit protos. */
public class CompilationUnits {
  private static final HashFunction DIGEST = Hashing.sha256();
  private static final Charset CHARSET = StandardCharsets.UTF_8;
  private static final byte[] TAG_CU = "CU".getBytes(CHARSET);
  private static final byte[] TAG_ARG = "ARG".getBytes(CHARSET);
  private static final byte[] TAG_CTX = "CTX".getBytes(CHARSET);
  private static final byte[] TAG_RI = "RI".getBytes(CHARSET);
  private static final byte[] TAG_IN = "IN".getBytes(CHARSET);
  private static final byte[] TAG_OUT = "OUT".getBytes(CHARSET);
  private static final byte[] TAG_SRC = "SRC".getBytes(CHARSET);
  private static final byte[] TAG_CWD = "CWD".getBytes(CHARSET);
  private static final byte[] TAG_DET = "DET".getBytes(CHARSET);
  private static final byte[] TAG_ENV = "ENV".getBytes(CHARSET);

  /**
   * Computes the standard digest for the specified {@link CompilationUnit} in stringified
   * hexidecimal form.
   */
  public static String digestFor(CompilationUnit compilationUnit) {
    CompilationUnit unit = canonicalize(compilationUnit);
    ByteArrayDataOutput w = ByteStreams.newDataOutput();
    putv(w, TAG_CU, compilationUnit.getVName());
    for (FileInput ri : compilationUnit.getRequiredInputList()) {
      putv(w, TAG_RI, ri.getVName());
      put(w, TAG_IN, ri.getInfo().getPath(), ri.getInfo().getDigest());
    }
    put(w, TAG_ARG, unit.getArgumentList());
    put(w, TAG_OUT, unit.getOutputKey());
    put(w, TAG_SRC, unit.getSourceFileList());
    put(w, TAG_CWD, unit.getWorkingDirectory());
    put(w, TAG_CTX, unit.getEntryContext());
    for (Env env : unit.getEnvironmentList()) {
      put(w, TAG_ENV, env.getName(), env.getValue());
    }
    for (Any d : unit.getDetailsList()) {
      w.write(TAG_DET);
      w.writeByte(10);
      w.write(d.getTypeUrl().getBytes(CHARSET));
      w.writeByte(0);
      w.write(d.getValue().toByteArray());
      w.writeByte(0);
    }
    return DIGEST.hashBytes(w.toByteArray()).toString();
  }

  private static void putv(ByteArrayDataOutput w, byte[] tag, VName v) {
    put(w, tag, v.getSignature(), v.getCorpus(), v.getRoot(), v.getPath(), v.getLanguage());
  }

  private static void put(ByteArrayDataOutput w, byte[] tag, String... vals) {
    put(w, tag, Arrays.asList(vals));
  }

  private static void put(ByteArrayDataOutput w, byte[] tag, Iterable<String> vals) {
    w.write(tag);
    w.writeByte(10);
    for (String val : vals) {
      w.write(val.getBytes(CHARSET));
      w.writeByte(0);
    }
  }

  private static int compareInputs(FileInput left, FileInput right) {
    int r = left.getInfo().getDigest().compareTo(right.getInfo().getDigest());
    if (r != 0) {
      return r;
    }
    return left.getInfo().getPath().compareTo(right.getInfo().getPath());
  }

  private static List<FileInput> sortAndDedupe(List<FileInput> orig) {
    if (orig.size() <= 1) {
      return orig;
    }
    List<FileInput> ri = Lists.newArrayList(orig);
    Collections.sort(ri, (a, b) -> a.getInfo().getDigest().compareTo(b.getInfo().getDigest()));
    // Invariant: All elements at or before i are unique (no duplicates).
    int i = 0, j = 1;
    while (j < ri.size()) {
      // If ri[j] ≠ ri[i], it is a new element, not a duplicate.  Move it
      // next in sequence after i and advance i; this ensures we keep the
      // first occurrence among duplicates.
      //
      // Otherwise, ri[k] == ri[i] for i <= k <= j, i.e., we are scanning a
      // run of duplicates, and should leave i where it is.
      if (compareInputs(ri.get(i), ri.get(j)) != 0) {
        i++;
        if (i != j) {
          FileInput tmp = ri.get(i);
          ri.set(i, ri.get(j));
          ri.set(j, tmp);
        }
      }
      j++;
    }
    return ri.subList(0, i + 1);
  }

  static CompilationUnit canonicalize(CompilationUnit src) {
    CompilationUnit.Builder unit = src.toBuilder();
    {
      List<FileInput> orig = unit.getRequiredInputList();
      unit.clearRequiredInput();
      unit.addAllRequiredInput(sortAndDedupe(orig));
    }
    {
      List<String> orig = unit.getSourceFileList();
      unit.clearSourceFile();
      orig.stream().sorted().forEach(unit::addSourceFile);
    }
    {
      List<Any> orig = unit.getDetailsList();
      unit.clearDetails();
      orig.stream()
          .sorted((a, b) -> a.getTypeUrl().compareTo(b.getTypeUrl()))
          .forEach(unit::addDetails);
    }
    {
      List<Env> orig = unit.getEnvironmentList();
      unit.clearEnvironment();
      orig.stream()
          .sorted((a, b) -> a.getName().compareTo(b.getName()))
          .forEach(unit::addEnvironment);
    }
    return unit.build();
  }
}
