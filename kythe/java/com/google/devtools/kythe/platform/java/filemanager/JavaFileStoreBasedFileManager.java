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

package com.google.devtools.kythe.platform.java.filemanager;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.tools.FileObject;
import javax.tools.JavaFileObject;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;

/** {@link StandardJavaFileManager} backed by a {@link JavaFileStore}. */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public class JavaFileStoreBasedFileManager extends ForwardingStandardJavaFileManager {

  protected JavaFileStore javaFileStore;

  public JavaFileStoreBasedFileManager(
      JavaFileStore javaFileStore, StandardJavaFileManager fileManager) {
    super(fileManager);
    this.javaFileStore = javaFileStore;
  }

  protected Set<String> getSearchPaths(Location location) {
    Set<String> dirsToLookIn = new HashSet<>();
    Iterable<? extends File> paths = getLocation(location);
    if (paths != null) {
      for (File dir : paths) {
        dirsToLookIn.add(dir.getPath());
      }
    }
    return dirsToLookIn;
  }

  /**
   * Lists all files in a package, tries to find files known to this manager before directing to the
   * underlying filemanager.
   */
  @Override
  public Iterable<JavaFileObject> list(
      Location location, String packageName, Set<Kind> kinds, boolean recurse) throws IOException {
    Set<String> dirsToLookIn = getSearchPaths(location);
    Set<CustomJavaFileObject> matchingFiles =
        javaFileStore.list(packageName, kinds, dirsToLookIn, recurse);
    // Prefer source over class file if the source exists in the source path:
    if (location == StandardLocation.CLASS_PATH && kinds.contains(Kind.CLASS)) {
      Map<String, CustomJavaFileObject> classNameToFile = new HashMap<>();
      for (CustomJavaFileObject fileObject : matchingFiles) {
        classNameToFile.put(fileObject.getClassName(), fileObject);
      }
      Set<Kind> sourceKinds = new HashSet<>();
      sourceKinds.add(Kind.SOURCE);
      Set<CustomJavaFileObject> matchedSources =
          javaFileStore.list(
              packageName, sourceKinds, getSearchPaths(StandardLocation.SOURCE_PATH), recurse);
      for (CustomJavaFileObject source : matchedSources) {
        if (classNameToFile.containsKey(source.getClassName())) {
          classNameToFile.put(source.getClassName(), source);
        }
      }
      matchingFiles.clear();
      matchingFiles.addAll(classNameToFile.values());
    }

    // TODO(schroederc): handle case where some of the package is in the JavaFileStore, but not all
    if (!matchingFiles.isEmpty()) {
      return new ArrayList<>(matchingFiles);
    }

    List<JavaFileObject> matchedFiles = new ArrayList<>();
    matchedFiles.addAll(matchingFiles);

    if (location == StandardLocation.SOURCE_PATH) {
      // XXX(#818): do not search the underlying file manager (and the local fs) for source files
      return matchedFiles;
    }

    // Ask underlying file manager for its knowledge of files, e.g.
    // in case of JRE we use the files locally known to the compiler.
    for (JavaFileObject jfo : super.list(location, packageName, kinds, recurse)) {
      matchedFiles.add(jfo);
    }
    return matchedFiles;
  }

  private JavaFileObject getJavaFileForInputFromCustomLocation(
      final Location location, final String className, final Kind kind) {
    return javaFileStore.find(className, kind, getSearchPaths(location));
  }

  /**
   * Gets file for classname, from local filemanager before directing to the underlying filemanager.
   */
  @Override
  public JavaFileObject getJavaFileForInput(
      final Location location, final String className, final Kind kind) throws IOException {
    JavaFileObject customJavaFile =
        getJavaFileForInputFromCustomLocation(location, className, kind);
    if (customJavaFile != null) {
      return customJavaFile;
    }
    return super.getJavaFileForInput(location, className, kind);
  }

  @Override
  public String inferBinaryName(Location location, JavaFileObject file) {
    if (file instanceof CustomJavaFileObject) {
      CustomJavaFileObject cjfo = (CustomJavaFileObject) file;
      return cjfo.getClassName();
    }
    return super.inferBinaryName(location, file);
  }

  public JavaFileObject getJavaFileFromPath(String file, Kind kind) {
    return javaFileStore.findByPath(file, kind);
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromFiles(
      Iterable<? extends File> files) {
    List<JavaFileObject> javaFileObjects = new ArrayList<>();
    for (File file : files) {
      String path = file.getPath();
      JavaFileObject.Kind kind = getKind(path);
      JavaFileObject jfo = getJavaFileFromPath(path, kind);
      javaFileObjects.add(jfo);
    }
    return javaFileObjects;
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(File... files) {
    return getJavaFileObjectsFromFiles(Arrays.asList(files));
  }

  @Override
  public FileObject getFileForInput(Location location, String packageName, String relativeName)
      throws IOException {
    FileObject fileObject = javaFileStore.find(packageName, relativeName, getSearchPaths(location));
    if (fileObject == null) {
      fileObject = super.getFileForInput(location, packageName, relativeName);
    }
    return fileObject;
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromStrings(Iterable<String> names) {
    List<File> files = new ArrayList<>();
    for (String name : names) {
      files.add(new File(name));
    }
    return getJavaFileObjectsFromFiles(files);
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(String... names) {
    List<File> files = new ArrayList<>();
    for (String name : names) {
      files.add(new File(name));
    }
    return getJavaFileObjectsFromFiles(files);
  }

  @Override
  public boolean contains(Location location, FileObject file) throws IOException {
    if (file instanceof CustomFileObject) {
      // The URI paths for custom file objects will always include an
      // extra leading '/'.
      String filePath = file.toUri().getPath().substring(1);
      // Do a trivial prefix-search based on the search paths for the given location.
      Set<String> paths = getSearchPaths(location);
      for (String path : paths) {
        if (filePath.startsWith(path)) {
          return true;
        }
      }
      return false;
    }
    return super.contains(location, file);
  }

  @Override
  public Path asPath(FileObject file) {
    if (file instanceof CustomFileObject) {
      return ((CustomFileObject) file).path;
    }
    return super.asPath(file);
  }

  public static Kind getKind(String name) {
    if (name.endsWith(Kind.CLASS.extension)) {
      return Kind.CLASS;
    } else if (name.endsWith(Kind.SOURCE.extension)) {
      return Kind.SOURCE;
    } else if (name.endsWith(Kind.HTML.extension)) {
      return Kind.HTML;
    } else {
      return Kind.OTHER;
    }
  }

  @Override
  public boolean isSameFile(FileObject a, FileObject b) {
    if (a instanceof CustomFileObject) {
      return a.equals(b);
    } else if (b instanceof CustomFileObject) {
      return b.equals(a);
    }
    return super.isSameFile(a, b);
  }
}
