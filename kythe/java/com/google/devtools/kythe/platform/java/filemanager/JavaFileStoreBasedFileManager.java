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

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.tools.FileObject;
import javax.tools.ForwardingJavaFileManager;
import javax.tools.JavaFileObject;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;

/**
 * TODO(schroederc) Refactor this code to not use Forwarding to avoid picking up spurious files from
 * the local file system. We do this today as we need to pick up the JDK etc...
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public class JavaFileStoreBasedFileManager
    extends ForwardingJavaFileManager<StandardJavaFileManager> implements StandardJavaFileManager {

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
    if (matchingFiles.size() > 0) {
      return new ArrayList<JavaFileObject>(matchingFiles);
    }

    List<JavaFileObject> matchedFiles = new ArrayList<>();
    matchedFiles.addAll(matchingFiles);
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
    return fileManager.inferBinaryName(location, file);
  }

  private JavaFileObject getJavaFileFromPath(String file, Kind kind) {
    return javaFileStore.findByPath(file, kind);
  }

  @Override
  public JavaFileObject getJavaFileForOutput(
      Location location, String className, JavaFileObject.Kind kind, FileObject sibling)
      throws IOException {
    return fileManager.getJavaFileForOutput(location, className, kind, sibling);
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
  public Iterable<? extends File> getLocation(Location location) {
    return fileManager.getLocation(location);
  }

  @Override
  public void setLocation(Location location, Iterable<? extends File> path) throws IOException {
    fileManager.setLocation(location, path);
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
    return fileManager.isSameFile(a, b);
  }
}
