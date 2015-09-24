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

package com.google.devtools.kythe.util;

import java.io.IOException;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.attribute.BasicFileAttributes;

/** DeleteRecursively is a {@link SimpleFileVisitor} that deletes every visited file/directory. */
public class DeleteRecursively extends SimpleFileVisitor<Path> {
  private static final DeleteRecursively INSTANCE = new DeleteRecursively();

  private DeleteRecursively() {}

  /** Deletes {@code path}, and if a {@code path} is a directory, all contained files. */
  public static void delete(Path path) throws IOException {
    Files.walkFileTree(path, INSTANCE);
  }

  @Override
  public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
    return delete(dir, exc);
  }

  @Override
  public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
    return delete(file, null);
  }

  private static FileVisitResult delete(Path p, IOException exc) throws IOException {
    if (exc != null) {
      throw exc;
    }
    Files.delete(p);
    return FileVisitResult.CONTINUE;
  }
}
