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

import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import javax.tools.JavaFileObject.Kind;

/** An implementation of {@link JavaFileStore} based on data from a {@link CompilationUnit}. */
public class CompilationUnitBasedJavaFileStore implements JavaFileStore {
  private final CompilationUnitFileTree fileTree;
  private FileDataProvider contentProvider;
  private Charset encoding;

  public CompilationUnitBasedJavaFileStore(
      CompilationUnit unit, FileDataProvider contentProvider, Charset encoding) {
    this.contentProvider = contentProvider;
    this.encoding = encoding;

    fileTree = new CompilationUnitFileTree(unit.getRequiredInputList());
  }

  @Override
  public CustomJavaFileObject find(String className, Kind kind, Set<String> pathPrefixes) {
    Path file = Paths.get(className.replace('.', '/') + kind.extension);
    String dirname = file.getParent().toString();
    String basename = file.getFileName().toString();
    for (String prefix : pathPrefixes) {
      String dirToLookIn = join(prefix, dirname);
      String digest = fileTree.lookup(dirToLookIn, basename);
      if (digest != null) {
        return new CustomJavaFileObject(
            contentProvider, join(prefix, file.toString()), digest, className, kind, encoding);
      }
    }
    return null;
  }

  @Override
  public CustomJavaFileObject findByPath(String path, Kind kind) {
    String digest = fileTree.lookup(path);
    if (digest != null) {
      return new CustomJavaFileObject(contentProvider, path, digest, null, kind, encoding);
    }
    return null;
  }

  @Override
  public CustomFileObject find(String packageName, String relativeName, Set<String> pathPrefixes) {
    String packagePath = packageName.replace('.', '/');
    for (String prefix : pathPrefixes) {
      String dirToLookIn = Paths.get(prefix, packagePath).toString();
      String digest = fileTree.lookup(dirToLookIn, relativeName);
      if (digest != null) {
        return new CustomFileObject(
            contentProvider, join(prefix, packagePath, relativeName), digest, encoding);
      }
    }
    return null;
  }

  /**
   * {@inheritDoc}
   *
   * TODO: recurse option does not work yet. This method only returns the classes inside
   * {@code packageName} and ignores the sub-packages.
   */
  @Override
  public Set<CustomJavaFileObject> list(
      String packageName, Set<Kind> kinds, Set<String> pathPrefixes, boolean recurse) {
    Set<CustomJavaFileObject> matchingFiles = new HashSet<>();
    if (pathPrefixes == null) {
      return matchingFiles;
    }
    String packagePath = packageName.replace('.', '/');
    for (String prefix : pathPrefixes) {
      String dirToLookIn = join(prefix, packagePath);
      Map<String, String> dir = fileTree.list(dirToLookIn);
      if (dir != null) {
        matchingFiles.addAll(getFiles(dirToLookIn, dir, kinds, packageName));
      }
    }
    return matchingFiles;
  }

  /** Uses the map built from the required inputs to build a list of files per directory. */
  private Set<CustomJavaFileObject> getFiles(
      String dirToLookIn, Map<String, String> entries, Set<Kind> kinds, String packageName) {
    Set<CustomJavaFileObject> files = new HashSet<>();
    for (Entry<String, String> entry : entries.entrySet()) {
      String fileName = entry.getKey();
      for (Kind kind : kinds) {
        if (fileName.endsWith(kind.extension)) {
          int lastDot = fileName.lastIndexOf('.');
          String clsName = packageName + "." + fileName.substring(0, lastDot);
          files.add(
              new CustomJavaFileObject(
                  contentProvider,
                  join(dirToLookIn, entry.getKey()),
                  entry.getValue(),
                  clsName,
                  kind,
                  encoding));
          break;
        }
      }
    }
    return files;
  }

  private static String join(String root, String... paths) {
    return Paths.get(root, paths).toString();
  }
}
