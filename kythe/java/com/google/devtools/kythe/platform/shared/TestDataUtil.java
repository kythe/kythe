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
package com.google.devtools.kythe.platform.shared;

import java.io.File;

public final class TestDataUtil {

  // This must match the name from the workspace(name={name}) rule in the root WORKSPACE file.
  private static final String DEFAULT_WORKSPACE = "io_kythe";

  /**
   * Load a test file from one of the files provided to the test. See the BUILD target of the test
   * for the list of files that can be accessed.
   */
  public static File getTestFile(String name) {
    File testDataRoot = new File(TestDataUtil.getTestRoot(), "kythe/testdata/platform");
    return new File(testDataRoot, name);
  }

  private static File getTestRoot() {
    String testSrcDir = System.getenv("TEST_SRCDIR");
    if (testSrcDir == null) {
      throw new RuntimeException("TEST_SRCDIR cannot be null when executing tests.");
    }
    String workspace = System.getenv("TEST_WORKSPACE");
    if (workspace == null) {
      workspace = DEFAULT_WORKSPACE;
    }
    return new File(testSrcDir, workspace);
  }
}
