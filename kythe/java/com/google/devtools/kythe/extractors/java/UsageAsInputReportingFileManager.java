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

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Iterables;
import com.google.common.flogger.FluentLogger;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URI;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import javax.tools.FileObject;
import javax.tools.ForwardingJavaFileManager;
import javax.tools.JavaFileObject;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardJavaFileManager;

/**
 * Wraps the StandardJavaFileManager to track which .java and .class files Javac touches for a given
 * compilation.
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
class UsageAsInputReportingFileManager extends ForwardingJavaFileManager<StandardJavaFileManager>
    implements StandardJavaFileManager {

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  // TODO(shahms): Remove these when we've moved to JDK9 and can invoke the methods directly.
  //  Until then, cache the lookup of these extended StandardJavaFileManager methods.
  private static final Method getLocationForModuleMethod =
      getMethodOrNull("getLocationForModule", Location.class, JavaFileObject.class);
  private static final Method containsMethod =
      getMethodOrNull("contains", Location.class, FileObject.class);
  private static final Method getJavaFileObjectsFromPathsMethod =
      getMethodOrNull("getJavaFileObjectsFromPaths", Iterable.class);
  private static final Method setLocationFromPathsMethod =
      getMethodOrNull("setLocationFromPaths", Location.class, Collection.class);
  private static final Method setLocationForModuleMethod =
      getMethodOrNull("setLocationForModule", Location.class, String.class, Collection.class);
  private static final Method asPathMethod = getMethodOrNull("asPath", FileObject.class);

  private final Map<URI, InputUsageRecord> inputUsageRecords = new HashMap<>();

  protected UsageAsInputReportingFileManager(StandardJavaFileManager fileManager) {
    super(fileManager);
  }

  /** Returns collection of JavaFileObjects that Javac read the contents of. */
  public Collection<InputUsageRecord> getUsages() {
    Collection<InputUsageRecord> result = new ArrayList<>();
    for (InputUsageRecord usageRecord : inputUsageRecords.values()) {
      if (usageRecord.isUsed()) {
        result.add(usageRecord);
      }
    }
    return result;
  }

  @Override
  public String inferBinaryName(Location location, JavaFileObject file) {
    return fileManager.inferBinaryName(location, unwrap(file));
  }

  @Override
  public Iterable<JavaFileObject> list(
      Location location, String packageName, Set<Kind> kinds, boolean recurse) throws IOException {
    return Iterables.transform(
        fileManager.list(location, packageName, kinds, recurse), input -> map(input, location));
  }

  /** Wraps a JavaFileObject in a UsageAsInputReportingJavaFileObject, shares existing instances. */
  private JavaFileObject map(JavaFileObject item, Location location) {
    if (item == null) {
      return item;
    }
    InputUsageRecord usage =
        inputUsageRecords.computeIfAbsent(item.toUri(), k -> new InputUsageRecord(item, location));
    return new UsageAsInputReportingJavaFileObject(item, usage);
  }

  /** Helper to match loading source files and tracking their usage. */
  public Iterable<JavaFileObject> getJavaFileForSources(Iterable<String> sources) {
    return Iterables.transform(
        fileManager.getJavaFileObjectsFromStrings(sources), input -> map(input, null));
  }

  @Override
  public JavaFileObject getJavaFileForInput(
      final Location location, final String className, final Kind kind) throws IOException {
    return map(fileManager.getJavaFileForInput(location, className, kind), location);
  }

  @Override
  public JavaFileObject getJavaFileForOutput(
      Location location, String className, Kind kind, FileObject sibling) throws IOException {
    // A java file opened initially for output might later get reopened for input (e.g.,
    // source files generated during annotation processing), so we need to track them too.
    return map(
        fileManager.getJavaFileForOutput(location, className, kind, unwrap(sibling)), location);
  }

  @Override
  public FileObject getFileForOutput(
      Location location, String packageName, String relativeName, FileObject sibling)
      throws IOException {
    return fileManager.getFileForOutput(location, packageName, relativeName, unwrap(sibling));
  }

  @Override
  public boolean isSameFile(FileObject a, FileObject b) {
    return super.isSameFile(unwrap(a), unwrap(b));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(String... names) {
    return getJavaFileForSources(ImmutableList.copyOf(names));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(File... files) {
    return getJavaFileObjectsFromFiles(ImmutableList.copyOf(files));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromStrings(Iterable<String> names) {
    return getJavaFileForSources(names);
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromFiles(
      Iterable<? extends File> files) {
    return Iterables.transform(
        fileManager.getJavaFileObjectsFromFiles(files), input -> map(input, null));
  }

  @Override
  public void setLocation(Location location, Iterable<? extends File> path) throws IOException {
    fileManager.setLocation(location, path);
  }

  @Override
  public Iterable<? extends File> getLocation(Location location) {
    return fileManager.getLocation(location);
  }

  // TODO(shahms): @Override; added in JDK9
  public Location getLocationForModule(Location location, JavaFileObject fo) throws IOException {
    // TODO(shahms): return fileManager.getLocationForModule(location, unwrap(fo));
    try {
      return (Location) getLocationForModuleMethod.invoke(fileManager, location, unwrap(fo));
    } catch (IllegalAccessException | InvocationTargetException e) {
      throw new IllegalStateException("getLocationForModule called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public boolean contains(Location location, FileObject fo) throws IOException {
    // TODO(shahms): return fileManager.contains(location, unwrap(fo));
    try {
      return (Boolean) containsMethod.invoke(fileManager, location, unwrap(fo));
    } catch (IllegalAccessException | InvocationTargetException e) {
      throw new IllegalStateException("contains called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromPaths(
      Collection<? extends Path> paths) {
    // TODO(shahms): return Iterables.transform(
    //      fileManager.getJavaFileObjectsFromPaths(paths), input -> map(input, null));
    try {
      return Iterables.transform(
          (Iterable<? extends JavaFileObject>)
              getJavaFileObjectsFromPathsMethod.invoke(fileManager, paths),
          input -> map(input, null));
    } catch (IllegalAccessException | InvocationTargetException e) {
      throw new IllegalStateException(
          "getJavaFileObjectsFromPaths called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public void setLocationFromPaths(Location location, Collection<? extends Path> paths)
      throws IOException {
    // TODO(shahms): fileManager.setLocationFromPaths(location, paths);
    try {
      setLocationFromPathsMethod.invoke(fileManager, location, paths);
    } catch (IllegalAccessException | InvocationTargetException e) {
      throw new IllegalStateException("setLocationFromPaths called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public void setLocationForModule(
      Location location, String moduleName, Collection<? extends Path> paths) throws IOException {
    // TODO(shahms): fileManager.setLocationForModule(location, moduleName, paths);
    try {
      setLocationForModuleMethod.invoke(fileManager, location, moduleName, paths);
    } catch (IllegalAccessException | InvocationTargetException e) {
      throw new IllegalStateException("setLocationForModule called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public Path asPath(FileObject fo) {
    // TODO(shahms): return fileManager.asPath(unwrap(fo));
    try {
      return (Path) asPathMethod.invoke(fileManager, unwrap(fo));
    } catch (IllegalAccessException | InvocationTargetException e) {
      throw new IllegalStateException("asPath called by unsupported Java version", e);
    }
  }

  // StandardJavaFileManager doesn't like it when it's asked about a JavaFileObject
  // it didn't create, so we need to unwrap our objects.
  private static FileObject unwrap(FileObject jfo) {
    if (jfo instanceof UsageAsInputReportingJavaFileObject) {
      return ((UsageAsInputReportingJavaFileObject) jfo).underlyingFileObject;
    }
    return jfo;
  }

  private static JavaFileObject unwrap(JavaFileObject jfo) {
    if (jfo instanceof UsageAsInputReportingJavaFileObject) {
      return ((UsageAsInputReportingJavaFileObject) jfo).underlyingFileObject;
    }
    return jfo;
  }

  private static Method getMethodOrNull(String name, Class<?>... parameterTypes) {
    try {
      return StandardJavaFileManager.class.getMethod(name, parameterTypes);
    } catch (NoSuchMethodException e) {
      logger.atInfo().log("Failed to find extended StandardJavaFileManager method: %s", e);
    }
    return null;
  }
}
