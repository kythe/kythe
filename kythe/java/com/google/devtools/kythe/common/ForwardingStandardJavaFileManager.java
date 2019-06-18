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
package com.google.devtools.kythe.common;

import com.google.common.flogger.FluentLogger;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.nio.file.Path;
import java.util.Collection;
import java.util.ServiceLoader;
import java.util.Set;
import javax.tools.FileObject;
import javax.tools.ForwardingJavaFileManager;
import javax.tools.JavaFileObject;
import javax.tools.StandardJavaFileManager;

/**
 * Forwards the full suite of {@link StandardJavaFileManager} methods to an underlying {@link
 * StandardJavaFileManager}, including methods introduced in JDK9 (except setPathFactory).
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public class ForwardingStandardJavaFileManager
    extends ForwardingJavaFileManager<StandardJavaFileManager> implements StandardJavaFileManager {

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  // TODO(shahms): Remove these when we've moved to JDK9 and can invoke the methods directly.
  //  Until then, cache the lookup of these extended StandardJavaFileManager methods.
  private static final Method getLocationForModuleNameMethod =
      getMethodOrNull("getLocationForModule", Location.class, String.class);
  private static final Method getLocationForModuleFileMethod =
      getMethodOrNull("getLocationForModule", Location.class, JavaFileObject.class);
  private static final Method getServiceLoaderMethod =
      getMethodOrNull("getServiceLoader", Location.class, Class.class);
  private static final Method inferModuleNameMethod =
      getMethodOrNull("inferModuleName", Location.class);
  private static final Method listLocationsForModulesMethod =
      getMethodOrNull("listLocationsForModules", Location.class);
  private static final Method containsMethod =
      getMethodOrNull("contains", Location.class, FileObject.class);
  private static final Method getJavaFileObjectsFromPathsMethod =
      getMethodOrNull("getJavaFileObjectsFromPaths", Iterable.class);
  private static final Method getJavaFileObjectsMethod =
      getMethodOrNull("getJavaFileObjects", Path[].class);
  private static final Method setLocationFromPathsMethod =
      getMethodOrNull("setLocationFromPaths", Location.class, Collection.class);
  private static final Method setLocationForModuleMethod =
      getMethodOrNull("setLocationForModule", Location.class, String.class, Collection.class);
  private static final Method getLocationAsPathsMethod =
      getMethodOrNull("getLocationAsPaths", Location.class);
  private static final Method asPathMethod = getMethodOrNull("asPath", FileObject.class);

  protected ForwardingStandardJavaFileManager(StandardJavaFileManager fileManager) {
    super(fileManager);
  }

  // TODO(shahms): @Override; added in JDK9
  public Location getLocationForModule(Location location, String moduleName) throws IOException {
    // TODO(shahms): return fileManager.getLocationForModule(location, fo);
    try {
      return (Location) getLocationForModuleNameMethod.invoke(fileManager, location, moduleName);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("getLocationForModule called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public Location getLocationForModule(Location location, JavaFileObject fo) throws IOException {
    // TODO(shahms): return fileManager.getLocationForModule(location, fo);
    try {
      return (Location) getLocationForModuleFileMethod.invoke(fileManager, location, fo);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("getLocationForModule called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  @SuppressWarnings({"unchecked"}) // safe by specification.
  public <S> ServiceLoader<S> getServiceLoader(Location location, Class<S> service)
      throws IOException {
    // TODO(shahms): return fileManager.getServiceLoader(location, service);
    try {
      return (ServiceLoader<S>) getServiceLoaderMethod.invoke(location, service);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("getServiceLoader called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public String inferModuleName(Location location) throws IOException {
    // TODO(shahms): return fileManager.inferModuleName(location);
    try {
      return (String) inferModuleNameMethod.invoke(fileManager, location);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("inferModuleName called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  @SuppressWarnings({"unchecked"}) // safe by specification.
  public Iterable<Set<Location>> listLocationsForModules(Location location) throws IOException {
    // TODO(shahms): return fileManager.listLocationsForModules(location);
    try {
      return (Iterable<Set<Location>>) listLocationsForModulesMethod.invoke(fileManager, location);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException(
          "listLocationsForModules called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public boolean contains(Location location, FileObject fo) throws IOException {
    // TODO(shahms): return fileManager.contains(location, fo);
    try {
      return (Boolean) containsMethod.invoke(fileManager, location, fo);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("contains called by unsupported Java version", e);
    }
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromFiles(
      Iterable<? extends File> files) {
    return fileManager.getJavaFileObjectsFromFiles(files);
  }

  // TODO(shahms): @Override; added in JDK9
  @SuppressWarnings({"unchecked", "IterablePathParameter"}) // safe by specification.
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromPaths(
      Iterable<? extends Path> paths) {
    //  TODO(shahms): fileManager.getJavaFileObjectsFromPaths(paths);
    try {
      return (Iterable<? extends JavaFileObject>)
          getJavaFileObjectsFromPathsMethod.invoke(fileManager, paths);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException(
          "getJavaFileObjectsFromPaths called by unsupported Java version", e);
    }
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromStrings(Iterable<String> names) {
    return fileManager.getJavaFileObjectsFromStrings(names);
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(File... files) {
    return fileManager.getJavaFileObjects(files);
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(String... names) {
    return fileManager.getJavaFileObjects(names);
  }

  // TODO(shahms): @Override; added in JDK9
  @SuppressWarnings({"unchecked"}) // safe by specification.
  public Iterable<? extends JavaFileObject> getJavaFileObjects(Path... paths) {
    //  TODO(shahms): fileManager.getJavaFileObjects(paths);
    try {
      return (Iterable<? extends JavaFileObject>)
          getJavaFileObjectsMethod.invoke(fileManager, (Object) paths);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("getJavaFileObjects called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public void setLocationFromPaths(Location location, Collection<? extends Path> paths)
      throws IOException {
    // TODO(shahms): fileManager.setLocationFromPaths(location, paths);
    try {
      setLocationFromPathsMethod.invoke(fileManager, location, paths);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("setLocationFromPaths called by unsupported Java version", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public void setLocationForModule(
      Location location, String moduleName, Collection<? extends Path> paths) throws IOException {
    // TODO(shahms): fileManager.setLocationForModule(location, moduleName, paths);
    try {
      setLocationForModuleMethod.invoke(fileManager, location, moduleName, paths);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("setLocationForModule called by unsupported Java version", e);
    }
  }

  @Override
  public Iterable<? extends File> getLocation(Location location) {
    return fileManager.getLocation(location);
  }

  // TODO(shahms): @Override; added in JDK9
  @SuppressWarnings({"unchecked"}) // Safe by specification.
  public Iterable<? extends Path> getLocationAsPaths(Location location) {
    // TODO(shahms): return fileManager.getLocationAsPaths(location);
    try {
      return (Iterable<? extends Path>) getLocationAsPathsMethod.invoke(fileManager, location);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("getLocationAsPaths called by unsupported Java version", e);
    }
  }

  @Override
  public boolean isSameFile(FileObject a, FileObject b) {
    return fileManager.isSameFile(a, b);
  }

  @Override
  public void setLocation(Location location, Iterable<? extends File> files) throws IOException {
    fileManager.setLocation(location, files);
  }

  // TODO(shahms): @Override; added in JDK9
  public Path asPath(FileObject fo) {
    // TODO(shahms): return fileManager.asPath(fo);
    try {
      return (Path) asPathMethod.invoke(fileManager, fo);
    } catch (ReflectiveOperationException e) {
      throw new IllegalStateException("asPath called by unsupported Java version", e);
    }
  }

  private static Method getMethodOrNull(String name, Class<?>... parameterTypes) {
    try {
      return StandardJavaFileManager.class.getMethod(name, parameterTypes);
    } catch (NoSuchMethodException e) {
      logger.atInfo().withCause(e).log("Failed to find extended StandardJavaFileManager method");
    }
    return null;
  }
}
