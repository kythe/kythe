/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

import static com.google.common.base.Preconditions.checkNotNull;

import javax.tools.JavaFileManager.Location;
import javax.tools.JavaFileObject;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * A compilation with multiple rounds of annotation processing will create new file objects for each
 * round. This class records which input paths were used at any point in the compilation. One
 * instance is created for each unique input path.
 */
public class InputUsageRecord {

  private final JavaFileObject fileObject;
  private final Location location;

  private boolean isUsed = false;

  /** @param location of a class file object, or {@code null} for source files. */
  public InputUsageRecord(JavaFileObject fileObject, @Nullable Location location) {
    this.fileObject = checkNotNull(fileObject);
    this.location = location;
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

  /** Return the file's {@link Location}. */
  public Location location() {
    return location;
  }
}
