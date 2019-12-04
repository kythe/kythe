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

package com.google.devtools.kythe.extractors.java;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Iterators;
import com.google.devtools.kythe.platform.java.filemanager.ForwardingStandardJavaFileManager;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Set;
import java.util.stream.Stream;
import javax.tools.JavaFileManager;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;
import javax.tools.ToolProvider;

/** Routines for expanding and copying a Java `--system` directory. */
final class SystemExploder {

  private SystemExploder() {}

  // Copies the system modules from the specified --system directory to the outputPath.
  public static void copySystemModules(String systemDir, Path outputPath) throws IOException {
    try (Stream<Path> paths = walkSystemModules(systemDir)) {
      for (Path path : (Iterable<Path>) paths::iterator) {
        Files.copy(path, outputPath.resolve(path.subpath(0, path.getNameCount()).toString()));
      }
    }
  }

  public static void copySystemModules(String systemDir, String outputDir) throws IOException {
    copySystemModules(systemDir, Paths.get(outputDir));
  }

  // Returns a stream of paths from the specified --system directory, beginning with the appropriate
  // modules subdirectory.
  public static Stream<Path> walkSystemModules(String systemDir) throws IOException {
    StandardJavaFileManager fileManager =
        ToolProvider.getSystemJavaCompiler().getStandardFileManager(null, null, null);
    fileManager.handleOption("--system", Iterators.singletonIterator(systemDir));
    return walkSystemModules(wrap(fileManager));
  }

  public static void main(String[] args) {
    try {
      if (args.length == 2) {
        copySystemModules(args[0], args[1]);
      } else if (args.length == 1) {
        dumpSystemPaths(args[0]);
      } else {
        dumpSystemPaths();
      }
    } catch (IOException e) {
      System.err.println(e.toString());
    }
  }

  // In order to work around the lack of the required JDK9 methods in google3, wrap
  // StandardJavaFileManagers before use (for now).
  private static ForwardingStandardJavaFileManager wrap(StandardJavaFileManager fileManager) {
    if (fileManager instanceof ForwardingStandardJavaFileManager) {
      return (ForwardingStandardJavaFileManager) fileManager;
    }
    return new ForwardingStandardJavaFileManager(fileManager) {};
  }

  private static Stream<Path> walkSystemModules(ForwardingStandardJavaFileManager fileManager)
      throws IOException {
    JavaFileManager.Location systemLocation = StandardLocation.valueOf("SYSTEM_MODULES");
    boolean started = false;
    ImmutableList.Builder<Path> modules = new ImmutableList.Builder<>();
    for (Set<JavaFileManager.Location> locs : fileManager.listLocationsForModules(systemLocation)) {
      for (JavaFileManager.Location loc : locs) {
        for (Path dir : fileManager.getLocationAsPaths(loc)) {
          try (Stream<Path> stream = Files.walk(dir)) {
            for (Path path : (Iterable<Path>) stream::iterator) {
              // Ensure that we include the top-level "modules" directory.
              if (!started) {
                started = true;
                modules.add(path.getParent());
              }
              modules.add(path);
            }
          }
        }
      }
    }
    // TODO(shahms): make this a lazy stream.
    return modules.build().stream();
  }

  private static void dumpSystemPaths(ForwardingStandardJavaFileManager fileManager)
      throws IOException {
    try (Stream<Path> paths = walkSystemModules(fileManager)) {
      for (Path path : (Iterable<Path>) paths::iterator) {
        System.err.println(
            (path.isAbsolute() ? path.subpath(0, path.getNameCount()) : path).toString());
      }
    }
  }

  private static void dumpSystemPaths() throws IOException {
    dumpSystemPaths(
        wrap(ToolProvider.getSystemJavaCompiler().getStandardFileManager(null, null, null)));
  }

  private static void dumpSystemPaths(String systemDir) throws IOException {
    StandardJavaFileManager fileManager =
        ToolProvider.getSystemJavaCompiler().getStandardFileManager(null, null, null);
    fileManager.handleOption("--system", Iterators.singletonIterator(systemDir));
    dumpSystemPaths(wrap(fileManager));
  }
}
