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

import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.collect.Iterators;
import com.google.common.collect.Maps;
import com.google.common.flogger.FluentLogger;
import com.google.common.jimfs.Configuration;
import com.google.common.jimfs.Jimfs;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.MoreExecutors;
import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.platform.shared.filesystem.CompilationUnitFileSystem;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.protobuf.Any;
import com.google.protobuf.InvalidProtocolBufferException;
import com.sun.tools.javac.main.Option;
import com.sun.tools.javac.main.OptionHelper;
import java.io.File;
import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.FileSystem;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Stream;
import javax.tools.FileObject;
import javax.tools.JavaFileObject;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * StandardJavaFileManager which uses a CompilationUnitFileSystem for managing paths based on on the
 * paths provided in the CompilationUnit.
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public final class CompilationUnitPathFileManager extends ForwardingStandardJavaFileManager {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private final ReadAheadDataProvider readAheadProvider;
  private final CompilationUnitFileSystem fileSystem;
  private final FileSystem memFileSystem;
  private final ImmutableSet<String> defaultPlatformClassPath;
  // The path given to us that we are allowed to write in. This will be stored as an absolute
  // path.
  private final @Nullable Path temporaryDirectoryPrefix;
  // A temporary directory inside of temporaryDirectoryPrefix that we will use and delete when the
  // close method is called. This will be stored as an absolute path.
  private @Nullable Path temporaryDirectory;

  public CompilationUnitPathFileManager(
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider,
      StandardJavaFileManager fileManager,
      @Nullable Path temporaryDirectory) {
    super(fileManager);
    // Store the absolute path so we can safely do startsWith checks later.
    this.temporaryDirectoryPrefix =
        temporaryDirectory == null ? null : temporaryDirectory.toAbsolutePath();
    defaultPlatformClassPath =
        ImmutableSet.copyOf(
            Iterables.transform(
                // TODO(shahms): use getLocationAsPaths on Java 9
                fileManager.getLocation(StandardLocation.PLATFORM_CLASS_PATH),
                f -> f.toPath().normalize().toString()));

    readAheadProvider = new ReadAheadDataProvider(fileDataProvider);
    fileSystem = CompilationUnitFileSystem.create(compilationUnit, readAheadProvider);
    memFileSystem = Jimfs.newFileSystem(Configuration.unix());
    // When compiled for Java 9+ this is ambiguous, so disambiguate to the compatibility shim.
    setPathFactory((ForwardingStandardJavaFileManager.PathFactory) this::getPath);
    setLocations(
        findJavaDetails(compilationUnit)
            .map(details -> toLocationMap(details))
            .orElseGet(() -> logMissingDetailsMap()));
  }

  @Override
  public Iterable<JavaFileObject> list(
      Location location, String packageName, Set<Kind> kinds, boolean recurse) throws IOException {
    return readAhead(super.list(location, packageName, kinds, recurse));
  }

  @Override
  public JavaFileObject getJavaFileForInput(Location location, String className, Kind kind)
      throws IOException {
    return readAhead(super.getJavaFileForInput(location, className, kind));
  }

  @Override
  public JavaFileObject getJavaFileForOutput(
      Location location, String className, Kind kind, FileObject sibling) throws IOException {
    return readAhead(super.getJavaFileForOutput(location, className, kind, sibling));
  }

  @Override
  public FileObject getFileForInput(Location location, String packageName, String relativeName)
      throws IOException {
    return readAhead(super.getFileForInput(location, packageName, relativeName));
  }

  @Override
  public FileObject getFileForOutput(
      Location location, String packageName, String relativeName, FileObject sibling)
      throws IOException {
    return readAhead(super.getFileForOutput(location, packageName, relativeName, sibling));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromFiles(
      Iterable<? extends File> files) {
    return readAhead(super.getJavaFileObjectsFromFiles(files));
  }

  @Override
  @SuppressWarnings({"IterablePathParameter"})
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromPaths(
      Iterable<? extends Path> path) {
    return readAhead(super.getJavaFileObjectsFromPaths(path));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjectsFromStrings(Iterable<String> names) {
    return readAhead(super.getJavaFileObjectsFromStrings(names));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(File... files) {
    return getJavaFileObjectsFromFiles(Arrays.asList(files));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(Path... paths) {
    return getJavaFileObjectsFromPaths(Arrays.asList(paths));
  }

  @Override
  public Iterable<? extends JavaFileObject> getJavaFileObjects(String... names) {
    return getJavaFileObjectsFromStrings(Arrays.asList(names));
  }

  @Override
  public boolean handleOption(String current, Iterator<String> remaining) {
    if (Option.SYSTEM.matches(current)) {
      try {
        Option.SYSTEM.handleOption(
            new OptionHelper.GrumpyHelper(null) {
              @Override
              public void put(String name, String value) {}

              @Override
              public boolean handleFileManagerOption(Option unused, String value) {
                try {
                  setSystemOption(value);
                } catch (IOException exc) {
                  throw new IllegalArgumentException(exc);
                }
                return true;
              }
            },
            current,
            remaining);
      } catch (Option.InvalidValueException exc) {
        throw new IllegalArgumentException(exc);
      }
      return true;
    }
    return super.handleOption(current, remaining);
  }

  @Override
  public void close() throws IOException {
    super.close();
    fileSystem.close();
    memFileSystem.close();
    if (temporaryDirectory != null) {
      Files.walkFileTree(
          temporaryDirectory,
          new SimpleFileVisitor<Path>() {
            @Override
            public FileVisitResult visitFile(Path file, BasicFileAttributes attrs)
                throws IOException {
              Files.delete(file);
              return FileVisitResult.CONTINUE;
            }

            @Override
            public FileVisitResult postVisitDirectory(Path dir, IOException exc)
                throws IOException {
              Files.delete(dir);
              return FileVisitResult.CONTINUE;
            }
          });
    }
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

  /** Returns a default map of Location to Path. */
  private ImmutableMap.Builder<Location, Collection<Path>> defaultLocationMapBuilder() {
    return new ImmutableMap.Builder<Location, Collection<Path>>()
        // While output paths are generally removed by the extractor and aren't used by the
        // indexer, the compiler front end checks that -d is present in modular builds, so
        // subvert that check here by defaulting to an in-memory directory path.
        // We do this here rather than during option processing because we can't reliably detect
        // modular build options from the file manager.
        .put(StandardLocation.CLASS_OUTPUT, ImmutableList.of(classOutputPath()));
  }

  /** Logs that path handling will fall back to Javac's option parsing. */
  private Map<Location, Collection<Path>> logMissingDetailsMap() {
    // It's expected that extractors which use JavaDetails will remove the corresponding
    // arguments from the command line.  Those extractors which don't use JavaDetails
    // (or options not present in the details), will remain on the command line and be
    // parsed as normal, relying on getPath() to map into the compilation unit.
    logger.atInfo().log("Compilation missing JavaDetails; falling back to flag parsing");
    return defaultLocationMapBuilder().build();
  }

  /** Translates the JavaDetails locations into {@code Map<Location, Collection<Path>>} */
  private Map<Location, Collection<Path>> toLocationMap(JavaDetails details) {
    return Maps.filterValues(
        defaultLocationMapBuilder()
            .put(StandardLocation.CLASS_PATH, toPaths(details.getClasspathList()))
            // TODO(shahms): Use StandardLocation.MODULE_PATH directly on JDK9+
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
    // If this is a path underneath the temporary directory, use it. This is required for --system
    // flags to work correctly.
    Path local = Paths.get(path, rest);
    if (temporaryDirectory != null && local.toAbsolutePath().startsWith(temporaryDirectory)) {
      logger.atInfo().log("Using the filesystem for temporary path %s", local);
      return local;
    }
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
    return local;
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

  private void setSystemOption(String value) throws IOException {
    // There are three kinds of --system flags we need to support:
    //   1) Bundled system images, with a lib/jrt-fs.jar and lib/modules image.
    //   2) Exploded system images, where the modules live under a modules subdirectory.
    //   3) "none"; a special value which indicates no system modules should be used.
    // The first must reside in the filesystem as there are multifarious asserts and checks that
    // this is so.
    if (value.equals("none")) {
      super.handleOption("--system", Iterators.singletonIterator("none"));
      return;
    }
    Path sys = fileSystem.getPath(value).normalize();
    if (Files.exists(sys.resolve("lib").resolve("jrt-fs.jar"))) {
      if (temporaryDirectoryPrefix == null) {
        logger.atSevere().log(
            "Can't create temporary directory to store system module because no temporary"
                + " directory was provided");
        throw new IllegalArgumentException("temporary directory needed but not provided");
      }
      if (temporaryDirectory != null) {
        throw new IllegalStateException("Temporary directory set twice");
      }
      temporaryDirectory =
          Files.createTempDirectory(temporaryDirectoryPrefix, "kythe_java_indexer")
              .toAbsolutePath();
      Path systemRoot = Files.createDirectory(temporaryDirectory.resolve("system"));
      try (Stream<Path> stream = Files.walk(sys.resolve("lib"))) {
        for (Path path : (Iterable<Path>) stream::iterator) {
          Path p =
              Files.copy(
                  path,
                  systemRoot.resolve(
                      path.subpath(sys.getNameCount(), path.getNameCount()).toString()));
          logger.atInfo().log("Copied file to %s", p.toAbsolutePath());
        }
      }
      logger.atInfo().log("Setting system path to %s", systemRoot);
      super.handleOption("--system", Iterators.singletonIterator(systemRoot.toString()));
    } else if (Files.isDirectory(sys.resolve("modules"))) {
      // TODO(shahms): Due to a bug in both javac argument validation and location validation,
      // we have to manually enumerate the available modules and set them directly.
      Location systemLocation = StandardLocation.valueOf("SYSTEM_MODULES");
      try (DirectoryStream<Path> stream = Files.newDirectoryStream(sys.resolve("modules"))) {
        for (Path entry : stream) {
          setLocationForModule(
              systemLocation, entry.getFileName().toString(), ImmutableList.of(entry));
        }
      }
    } else {
      throw new IllegalArgumentException(value);
    }
  }

  private <T extends FileObject> T readAhead(T fo) {
    if (fo != null) {
      // Some of the methods we wrap will return null to indicate failure,
      // so handle that uniformly here rather than in each of those methods.
      readAhead(asPath(fo));
    }
    return fo;
  }

  private <T extends FileObject> Iterable<T> readAhead(Iterable<T> files) {
    // While Iterables.transform uses a lazy iterator rather than eagerly initiating readahead,
    // testing suggests it results in the biggest performance improvement.
    // Should that change in the future, this should be made an explicit for loop.
    return Iterables.transform(files, this::readAhead);
  }

  private void readAhead(Path path) {
    if (path.getFileSystem().equals(fileSystem)) {
      readAheadProvider.readAhead(path.toString(), path.toUri().getHost());
    }
  }

  /** Returns a path to a memory-backed temporary directory. */
  private Path classOutputPath() {
    try {
      return Files.createTempDirectory(memFileSystem.getPath("/"), "class-output.");
    } catch (IOException cause) {
      throw new IllegalStateException("Unable to create class output path", cause);
    }
  }

  private static class ReadAheadDataProvider implements FileDataProvider {
    final Map<String, ListenableFuture<byte[]>> cache = new ConcurrentHashMap<>();
    final FileDataProvider provider;

    ReadAheadDataProvider(FileDataProvider provider) {
      this.provider = provider;
    }

    @SuppressWarnings({"FutureReturnValueIgnored"})
    void readAhead(String path, String digest) {
      readCached(path, digest);
    }

    @Override
    public ListenableFuture<byte[]> startLookup(String path, String digest) {
      ListenableFuture<byte[]> result = readCached(path, digest);
      result.addListener(() -> cache.remove(digest), MoreExecutors.directExecutor());
      return result;
    }

    @Override
    public void close() throws Exception {
      provider.close();
    }

    private ListenableFuture<byte[]> readCached(String path, String digest) {
      Preconditions.checkArgument(!Strings.isNullOrEmpty(digest), "digest is empty");
      return cache.computeIfAbsent(digest, k -> provider.startLookup(path, digest));
    }
  }
}
