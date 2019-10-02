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

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.collect.Maps;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.platform.shared.filesystem.CompilationUnitFileSystem;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.protobuf.Any;
import com.google.protobuf.InvalidProtocolBufferException;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Collection;
import java.util.Map;
import java.util.Optional;
import javax.tools.JavaFileManager.Location;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;

/**
 * StandardJavaFileManager which uses a CompilationUnitFileSystem for managing paths based on on the
 * paths provided in the CompilationUnit.
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public final class CompilationUnitPathFileManager extends ForwardingStandardJavaFileManager {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private final CompilationUnitFileSystem fileSystem;
  private final ImmutableSet<String> defaultPlatformClassPath;

  public CompilationUnitPathFileManager(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      StandardJavaFileManager fileManager) {
    super(fileManager);
    defaultPlatformClassPath =
        ImmutableSet.copyOf(
            Iterables.transform(
                fileManager.getLocationAsPaths(StandardLocation.PLATFORM_CLASS_PATH),
                p -> p.normalize().toString()));

    fileSystem = CompilationUnitFileSystem.create(compilationUnit, fileDataProvider);
    // When compiled for Java 9+ this is ambiguous, so disambiguate to the compatibility shim.
    setPathFactory((ForwardingStandardJavaFileManager.PathFactory) this::getPath);
    setLocations(
        findJavaDetails(compilationUnit)
            .map(details -> toLocationMap(details))
            .orElseGet(() -> logEmptyLocationMap()));
  }

  /** Extracts the embedded JavaDetails message, if any, from the CompilationUnit. */
  private static Optional<JavaDetails> findJavaDetails(CompilationUnit unit) {
    for (Any details : unit.getDetailsList()) {
      try {
        if (details.getTypeUrl().equals(JavaCompilationUnitExtractor.JAVA_DETAILS_URL)) {
          return Optional.of(JavaDetails.parseFrom(details.getValue()));
        } else if (details.is(JavaDetails.class)) {
          return Optional.of(details.unpack(JavaDetails.class));
        }
      } catch (InvalidProtocolBufferException ipbe) {
        logger.atWarning().withCause(ipbe).log("error unpacking JavaDetails");
      }
    }
    return Optional.empty();
  }

  /** Logs that path handling will fall back to Javac's option parsing. */
  private static Map<Location, Collection<Path>> logEmptyLocationMap() {
    // It's expected that extractors which use JavaDetails will remove the corresponding
    // arguments from the command line.  Those extractors which don't use JavaDetails
    // (or options not present in the details), will remain on the command line and be
    // parsed as normal, relying on getPath() to map into the compilation unit.
    logger.atInfo().log("Compilation missing JavaDetails; falling back to flag parsing");
    return ImmutableMap.of();
  }

  /** Translates the JavaDetails locations into {@code Map<Location, Collection<Path>>} */
  private Map<Location, Collection<Path>> toLocationMap(JavaDetails details) {
    return Maps.filterValues(
        new ImmutableMap.Builder<Location, Collection<Path>>()
            .put(StandardLocation.CLASS_PATH, toPaths(details.getClasspathList()))
            .put(StandardLocation.locationFor("MODULE_PATH"), toPaths(details.getClasspathList()))
            .put(StandardLocation.SOURCE_PATH, toPaths(details.getSourcepathList()))
            .put(
                StandardLocation.PLATFORM_CLASS_PATH,
                // bootclasspath should fall back to the local filesystem,
                // while the others should only come from the compilation unit.
                toPaths(details.getBootclasspathList(), this::getPath))
            .build(),
        v -> !v.isEmpty());
  }

  private Collection<Path> toPaths(Collection<String> paths) {
    return toPaths(paths, fileSystem::getPath);
  }

  private Collection<Path> toPaths(Collection<String> paths, PathFactory factory) {
    return paths.stream().map(factory::getPath).collect(ImmutableList.toImmutableList());
  }

  private Path getPath(String path, String... rest) {
    // In order to support paths passed via command line options rather than
    // JavaDetails, prevent loading source files from outside the
    // CompilationUnit (#818), and allow JDK classes to be provided by the
    // platform we always form CompilationUnit paths unless that path is not
    // present in the CompilationUnit and is part of the default boot class path.
    Path result = fileSystem.getPath(path, rest);
    if (Files.exists(result) || !defaultPlatformClassPath.contains(result.normalize().toString())) {
      return result;
    }
    logger.atFine().log("Falling back to filesystem for %s", result);
    return Paths.get(path, rest);
  }

  /** For each entry in the provided map, sets the corresponding location in fileManager */
  private void setLocations(Map<Location, Collection<Path>> locations) {
    for (Map.Entry<Location, Collection<Path>> entry : locations.entrySet()) {
      try {
        setLocationFromPaths(entry.getKey(), entry.getValue());
      } catch (IOException ex) {
        logger.atWarning().withCause(ex).log("error setting location %s", entry);
      }
    }
  }
}
