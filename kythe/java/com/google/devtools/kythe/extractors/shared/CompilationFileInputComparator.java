/*
 * Copyright 2014 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.extractors.shared;

import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import java.util.Comparator;

/** {@code Comparator} for {@code FileInput}. */
public class CompilationFileInputComparator implements Comparator<FileInput> {
  private static final CompilationFileInputComparator COMPARATOR =
      new CompilationFileInputComparator();

  @Override
  public int compare(FileInput left, FileInput right) {
    return left.getInfo().getPath().compareTo(right.getInfo().getPath());
  }

  private CompilationFileInputComparator() {}

  public static CompilationFileInputComparator getComparator() {
    return COMPARATOR;
  }
}
