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

package com.google.devtools.kythe.extractors.java;

import javax.tools.JavaFileObject;

/**
 * A compilation with multiple rounds of annotation processing will create new file objects for each
 * round. This class records which input paths were used at any point in the compilation. One
 * instance is created for each unique input path.
 */
public class InputUsageRecord {

  private final JavaFileObject fileObject;
  private boolean isUsed = false;

  public InputUsageRecord(JavaFileObject fileObject) {
    if (fileObject == null) {
      throw new IllegalStateException();
    }
    this.fileObject = fileObject;
  }

  /** Record that the compiler used this file as input. */
  public void markUsed() {
    isUsed = true;
  }

  /** Returns true if the compiler used this file as input. */
  public boolean isUsed() {
    return isUsed;
  }

  /** Returns the first file object created for this file. */
  public JavaFileObject fileObject() {
    return fileObject;
  }
}
