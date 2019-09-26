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

  public CompilationUnitPathFileManager(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      StandardJavaFileManager fileManager) {
    super(fileManager);

    fileSystem = CompilationUnitFileSystem.create(compilationUnit, fileDataProvider);
    // When compiled for Java 9+ this is ambiguous, so disambiguate to the compatibility shim.
    setPathFactory((ForwardingStandardJavaFileManager.PathFactory) this::getPath);
    setLocations(
        findJavaDetails(compilationUnit)
            .map(details -> toLocationMap(details))
            .orElseGet(() -> toLocationMap(compilationUnit.getArgumentList())));
  }

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

  /** Translates the argument list locations into Map<Location, Collection<Path>> */
  private Map<Location, Collection<Path>> toLocationMap(Collection<String> arguments) {
    // TODO(shahms): Actually implement this.  Although, it's not critical as the compiler will
    // apply the options directly otherwise.
    logger.atWarning().log("Compilation missing JavaDetails; falling back to flag parsing");
    // classpath.add("");
    // classpath.addAll(getPathSet(unit.getArgumentList(), "-cp"));
    // classpath.addAll(getPathSet(unit.getArgumentList(), "-classpath"));
    // sourcepath.add("");
    // sourcepath.addAll(getPathSet(unit.getArgumentList(), "-sourcepath"));
    // bootclasspath.add("");
    // bootclasspath.addAll(getPathSet(unit.getArgumentList(), "-bootclasspath"));
    return ImmutableMap.of();
  }

  /** Translates the JavaDetails locations into Map<Location, Collection<Path>> */
  private Map<Location, Collection<Path>> toLocationMap(JavaDetails details) {
    return Maps.filterValues(
        new ImmutableMap.Builder<Location, Collection<Path>>()
            .put(StandardLocation.CLASS_PATH, toPaths(details.getClasspathList()))
            .put(StandardLocation.locationFor("MODULE_PATH"), toPaths(details.getClasspathList()))
            .put(StandardLocation.SOURCE_PATH, toPaths(details.getSourcepathList()))
            .put(
                StandardLocation.PLATFORM_CLASS_PATH,
                // bootclasspath should default to the local filesystem,
                // while the others should come from the compilation unit.
                toPaths(details.getBootclasspathList(), Paths::get))
            .build(),
        v -> !Iterables.isEmpty(v));
  }

  private Collection<Path> toPaths(Collection<String> paths) {
    return toPaths(paths, fileSystem::getPath);
  }

  private Collection<Path> toPaths(Collection<String> paths, PathFactory factory) {
    return paths.stream().map(factory::getPath).collect(ImmutableList.toImmutableList());
  }

  private Path getPath(String path, String... rest) {
    // TODO(shahms): This is something of a hack to work around the fact that:
    // 1) The bootclasspath typically comes from the default filesystem.
    // 2) We set bootclasspath from JavaDetails
    // 3) Then JavaCompilationDetails sets it using command line flags, which picks up
    //    the default path factory.
    Path result = fileSystem.getPath(path, rest);
    if (Files.exists(result)) {
      return result;
    }
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
