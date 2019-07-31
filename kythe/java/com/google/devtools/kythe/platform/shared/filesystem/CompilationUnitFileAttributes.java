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

package com.google.devtools.kythe.platform.shared.filesystem;

import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileTime;

/** {@link BasicFileAttributes} implementat for {@link CompilationUnitFileSystem}. */
final class CompilationUnitFileAttributes implements BasicFileAttributes {
  private final long size;

  CompilationUnitFileAttributes(long size) {
    this.size = size;
  }

  @Override
  public FileTime lastModifiedTime() {
    return FileTime.fromMillis(0);
  }

  @Override
  public FileTime lastAccessTime() {
    return lastModifiedTime();
  }

  @Override
  public FileTime creationTime() {
    return lastModifiedTime();
  }

  @Override
  public boolean isRegularFile() {
    return size >= 0;
  }

  @Override
  public boolean isDirectory() {
    return size < 0;
  }

  @Override
  public boolean isSymbolicLink() {
    return false;
  }

  @Override
  public boolean isOther() {
    return false;
  }

  @Override
  public long size() {
    return size;
  }

  @Override
  public Object fileKey() {
    return null;
  }
}
