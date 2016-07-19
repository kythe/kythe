/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.platform.java.filemanager;

import com.google.devtools.kythe.proto.Analysis;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import java.util.LinkedList;
import java.util.List;
import junit.framework.TestCase;

/** Tests {@link CompilationUnitFileTree}. */
public class CompilationUnitFileTreeTest extends TestCase {
  public void testRelPath() {
    Iterable<CompilationUnit.FileInput> fi = createDummyFileInput("foo/bar/baz");
    CompilationUnitFileTree cuft = new CompilationUnitFileTree(fi);
    assertEquals("dummy digest for foo/bar/baz", cuft.lookup("foo/bar/baz"));
    assertEquals("<dir>", cuft.lookup("foo/bar"));
  }

  public void testAbsPath() {
    Iterable<CompilationUnit.FileInput> fi = createDummyFileInput("/foo/bar/baz");
    CompilationUnitFileTree cuft = new CompilationUnitFileTree(fi);
    assertEquals("dummy digest for /foo/bar/baz", cuft.lookup("/foo/bar/baz"));
    assertEquals("<dir>", cuft.lookup("/foo/bar"));
  }

  private List<FileInput> createDummyFileInput(String path) {
    List<FileInput> ret = new LinkedList<>();
    ret.add(
        CompilationUnit.FileInput.newBuilder()
            .setInfo(
                Analysis.FileInfo.newBuilder()
                    .setPath(path)
                    .setDigest("dummy digest for " + path)
                    .build())
            .build());
    return ret;
  }
}
