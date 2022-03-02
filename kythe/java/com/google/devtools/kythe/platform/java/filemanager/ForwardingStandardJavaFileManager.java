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

import java.io.File;
import java.io.IOException;
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
@SuppressWarnings("MissingOverride")
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public class ForwardingStandardJavaFileManager
    extends ForwardingJavaFileManager<StandardJavaFileManager> implements StandardJavaFileManager {

  protected ForwardingStandardJavaFileManager(StandardJavaFileManager fileManager) {
    super(fileManager);
  }

  @Override
  public Location getLocationForModule(Location location, String moduleName) throws IOException {
    return fileManager.getLocationForModule(location, moduleName);
  }

  @Override
  public Location getLocationForModule(Location location, JavaFileObject fo) throws IOException {
    return fileManager.getLocationForModule(location, fo);
  }

  @Override
  public <S> ServiceLoader<S> getServiceLoader(Location location, Class<S> service)
      throws IOException {
    return fileManager.getServiceLoader(location, service);
  }

  @Override
  public String inferModuleName(Location location) throws IOException {
    return fileManager.inferModuleName(location);
  }

  @Override
  public Iterable<Set<Location>> listLocationsForModules(Location location) throws IOException {
    return fileManager.listLocationsForModules(location);
  }

  @Override
  public boolean contains(Location location, FileObject fo) throws IOException {
    return fileManager.contains(location, fo);
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromFiles(
      Iterable<? extends File> files) {
    return fileManager.getJavaFileObjectsFromFiles(files);
  }

  @Override
  @SuppressWarnings({"IterablePathParameter"}) // required by interface.
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromPaths(
      Iterable<? extends Path> paths) {
    return fileManager.getJavaFileObjectsFromPaths(paths);
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

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(Path... paths) {
    return fileManager.getJavaFileObjects(paths);
  }

  @Override
  public void setLocationFromPaths(Location location, Collection<? extends Path> paths)
      throws IOException {
    fileManager.setLocationFromPaths(location, paths);
  }

  @Override
  public void setLocationForModule(
      Location location, String moduleName, Collection<? extends Path> paths) throws IOException {
    fileManager.setLocationForModule(location, moduleName, paths);
  }

  @Override
  public Iterable<? extends File> getLocation(Location location) {
    return fileManager.getLocation(location);
  }

  @Override
  public Iterable<? extends Path> getLocationAsPaths(Location location) {
    return fileManager.getLocationAsPaths(location);
  }

  @Override
  public boolean isSameFile(FileObject a, FileObject b) {
    return fileManager.isSameFile(a, b);
  }

  @Override
  public void setLocation(Location location, Iterable<? extends File> files) throws IOException {
    fileManager.setLocation(location, files);
  }

  @Override
  public Path asPath(FileObject fo) {
    return fileManager.asPath(fo);
  }

  @Override
  public void setPathFactory(StandardJavaFileManager.PathFactory factory) {
    fileManager.setPathFactory(factory);
  }
}
