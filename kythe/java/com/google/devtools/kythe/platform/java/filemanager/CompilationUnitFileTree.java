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

package com.google.devtools.kythe.platform.java.filemanager;

import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

/** Represents the directory structure of the inputs of a compilation unit. */
public class CompilationUnitFileTree {
  /**
   * Map from a directory path to file entries in that directory. Each file entry is a pair of file
   * name and file's digest.
   */
  private Map<String, Map<String, String>> dirs = new HashMap<>();

  public static final String DIRECTORY_DIGEST = "<dir>";

  public CompilationUnitFileTree(Iterable<CompilationUnit.FileInput> inputs) {
    // Build a directory map from the list of required inputs.
    for (CompilationUnit.FileInput input : inputs) {
      String path = input.getInfo().getPath();
      String digest = input.getInfo().getDigest();

      Path curPath = Paths.get(path);
      putDigest(curPath, digest);

      while ((curPath = curPath.getParent()) != null && curPath.getFileName() != null) {
        putDigest(curPath, DIRECTORY_DIGEST);
      }
    }
  }

  // Note: Do not call on the absolute root path '/'.
  private void putDigest(Path path, String digest) {
    Path parent = path.getParent();
    String dirname;
    if (parent != null) {
      dirname = parent.toString();
    } else if (path.isAbsolute()) {
      // path.getFileName() below will return null on the path '/', leading to an NPE.
      throw new IllegalArgumentException("The path '/' cannot be used here.");
    } else {
      dirname = ".";
    }
    String basename = path.getFileName().toString();
    Map<String, String> dir = dirs.get(dirname);
    if (dir == null) {
      dir = new HashMap<>();
      dirs.put(dirname, dir);
    }
    String existing = dir.get(digest);
    if (existing != null && !existing.equals(digest)) {
      throw new IllegalStateException("Trying to register conflicting digests for the same path");
    }
    dir.put(basename, digest);
  }

  /**
   * Returns the digest of the file with the specified directory name and file name.
   *
   * @param directory the directory name.
   * @param filename the file name.
   * @return the digest or null if no such file is present.
   */
  public String lookup(String directory, String filename) {
    Map<String, String> dir = dirs.get(directory);
    if (dir != null) {
      return dir.get(filename);
    }
    return null;
  }

  /**
   * Returns the digest of the file with the specified path.
   *
   * @param path the path.
   * @return the digest or null if no such file is present.
   */
  public String lookup(String path) {
    Path p = Paths.get(path);
    Path parent = p.getParent();
    return lookup(parent == null ? null : parent.toString(), p.getFileName().toString());
  }

  /**
   * Returns the list of file names in the specified directory and their digests.
   *
   * @param dirToLookIn the directory name.
   * @return the map of filenames to digests, or null if no such directory exists.
   */
  public Map<String, String> list(String dirToLookIn) {
    return dirs.get(dirToLookIn);
  }
}
