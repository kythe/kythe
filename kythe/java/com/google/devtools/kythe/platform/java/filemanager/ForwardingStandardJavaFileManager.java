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
package com.google.devtools.kythe.platform.java.filemanager;

import com.google.common.base.Throwables;
import com.google.common.collect.Iterables;
import com.google.common.flogger.FluentLogger;
import com.google.common.reflect.Reflection;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
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
 * StandardJavaFileManager}, including methods introduced in JDK9.
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
  private static final Class<?> pathFactoryInterface = getClassOrNull("PathFactory");
  private static final Method setPathFactoryMethod =
      getMethodOrNull("setPathFactory", pathFactoryInterface);

  /** Equivalent interface for JDK9's StandardJavaFileManager.PathFactory */
  @FunctionalInterface
  public static interface PathFactory {
    Path getPath(String path, String... more);
  }

  protected ForwardingStandardJavaFileManager(StandardJavaFileManager fileManager) {
    super(fileManager);
  }

  // TODO(shahms): @Override; added in JDK9
  public Location getLocationForModule(Location location, String moduleName) throws IOException {
    // TODO(shahms): return fileManager.getLocationForModule(location, fo);
    try {
      return (Location) getLocationForModuleNameMethod.invoke(fileManager, location, moduleName);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("getLocationForModule", e, IOException.class);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public Location getLocationForModule(Location location, JavaFileObject fo) throws IOException {
    // TODO(shahms): return fileManager.getLocationForModule(location, fo);
    try {
      return (Location) getLocationForModuleFileMethod.invoke(fileManager, location, fo);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("getLocationForModule", e, IOException.class);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  @SuppressWarnings({"unchecked"}) // safe by specification.
  public <S> ServiceLoader<S> getServiceLoader(Location location, Class<S> service)
      throws IOException {
    // TODO(shahms): return fileManager.getServiceLoader(location, service);
    try {
      return (ServiceLoader<S>) getServiceLoaderMethod.invoke(location, service);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("getServiceLoader", e, IOException.class);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public String inferModuleName(Location location) throws IOException {
    // TODO(shahms): return fileManager.inferModuleName(location);
    try {
      return (String) inferModuleNameMethod.invoke(fileManager, location);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("inferModuleName", e, IOException.class);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  @SuppressWarnings({"unchecked"}) // safe by specification.
  public Iterable<Set<Location>> listLocationsForModules(Location location) throws IOException {
    // TODO(shahms): return fileManager.listLocationsForModules(location);
    try {
      return (Iterable<Set<Location>>) listLocationsForModulesMethod.invoke(fileManager, location);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible(
          "listLocationsForModules", e, IOException.class);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public boolean contains(Location location, FileObject fo) throws IOException {
    // TODO(shahms): return fileManager.contains(location, fo);
    try {
      return (Boolean) containsMethod.invoke(fileManager, location, fo);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("contains", e, IOException.class);
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
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("getJavaFileObjectsFromPaths", e);
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
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("getJavaFileObjects", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public void setLocationFromPaths(Location location, Collection<? extends Path> paths)
      throws IOException {
    // TODO(shahms): fileManager.setLocationFromPaths(location, paths);
    try {
      setLocationFromPathsMethod.invoke(fileManager, location, paths);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("setLocationFromPaths", e, IOException.class);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public void setLocationForModule(
      Location location, String moduleName, Collection<? extends Path> paths) throws IOException {
    // TODO(shahms): fileManager.setLocationForModule(location, moduleName, paths);
    try {
      setLocationForModuleMethod.invoke(fileManager, location, moduleName, paths);
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("setLocationForModule", e, IOException.class);
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
    } catch (NullPointerException npe) {
      // Fall back to using getLocation from the underlying fileManager.
      return Iterables.transform(fileManager.getLocation(location), File::toPath);
    } catch (ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("getLocationAsPaths", e);
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
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("asPath", e);
    }
  }

  // TODO(shahms): @Override; added in JDK9
  public void setPathFactory(PathFactory factory) {
    // TODO(shahms): fileManager.setPathFactory(factory);
    try {
      setPathFactoryMethod.invoke(
          fileManager,
          Reflection.newProxy(
              pathFactoryInterface,
              (proxy, method, args) -> {
                return factory.getPath((String) args[0], (String[]) args[1]);
              }));
    } catch (NullPointerException | ReflectiveOperationException e) {
      throw propagateInvocationTargetErrorIfPossible("setPathFactory", e);
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

  private static Class<?> getClassOrNull(String name) {
    try {
      return Class.forName(
          StandardJavaFileManager.class.getCanonicalName() + "$" + name,
          true,
          StandardJavaFileManager.class.getClassLoader());
    } catch (ClassNotFoundException e) {
      logger.atInfo().withCause(e).log("Failed to find StandardJavaFileManager class");
    }
    return null;
  }

  private static UnsupportedOperationException propagateInvocationTargetErrorIfPossible(
      String methodName, Throwable error) {
    if (error instanceof InvocationTargetException) {
      Throwables.throwIfUnchecked(((InvocationTargetException) error).getCause());
    }
    return unsupportedVersionError(methodName, error);
  }

  private static UnsupportedOperationException propagateInvocationTargetErrorIfPossible(
      String methodName, Throwable error, Class<IOException> declaredType) throws IOException {
    if (error instanceof InvocationTargetException) {
      // Log the exception because the propagated destination may not provide a nice error log.
      Throwable t = ((InvocationTargetException) error).getCause();
      logger.atWarning().withCause(t).log(
          "Error in underlying filemanager. A more detailed message may have been output to"
              + " stderr.");
      Throwables.propagateIfPossible(t, declaredType);
    }
    return unsupportedVersionError(methodName, error);
  }

  private static UnsupportedOperationException unsupportedVersionError(
      String methodName, Throwable cause) {
    return new UnsupportedOperationException(
        methodName + " called by unsupported Java version", cause);
  }
}
