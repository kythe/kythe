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

package com.google.devtools.kythe.platform.shared.filesystem;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.platform.java.filemanager.CompilationUnitFileTree;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.FileStore;
import java.nio.file.FileSystem;
import java.nio.file.FileSystems;
import java.nio.file.Path;
import java.nio.file.PathMatcher;
import java.nio.file.WatchService;
import java.nio.file.attribute.UserPrincipalLookupService;
import java.nio.file.spi.FileSystemProvider;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutionException;
import java.util.stream.Collectors;

/**
 * A {@link FileSystem} over a {@link CompilationUnit} and {@link FileDataProvider}.
 *
 * <p>Presents a view of a filesystem whose contents are those paths specified in the required_input
 * of the CompilationUnit and file contents are provided by the associated FileDataProvider.
 *
 * <p>The compilation working_directory is used as the filesystem root directory, against which
 * relative paths will be resolved.
 */
final class CompilationUnitFileSystem extends FileSystem {

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private static final Set<String> SUPPORTED_VIEWS = ImmutableSet.of("basic");

  private final CompilationUnitFileSystemProvider provider;
  private final CompilationUnitPath rootDirectory;
  private CompilationUnitFileTree compilationFileTree;
  private FileDataProvider fileDataProvider;
  private boolean closed = false;

  CompilationUnitFileSystem(
      CompilationUnitFileSystemProvider provider,
      CompilationUnit compilationUnit,
      FileDataProvider fileDataProvider) {
    checkNotNull(provider);
    checkNotNull(compilationUnit);
    checkNotNull(fileDataProvider);

    this.provider = provider;

    String workingDirectory = compilationUnit.getWorkingDirectory();
    this.rootDirectory =
        // Use the compilationUnit working directory as our root, if present.
        new CompilationUnitPath(this, workingDirectory.isEmpty() ? "/" : workingDirectory);

    this.compilationFileTree =
        new CompilationUnitFileTree(
            compilationUnit.getRequiredInputList().stream()
                .map(
                    fi -> {
                      CompilationUnit.FileInput.Builder builder = fi.toBuilder();
                      builder
                          .getInfoBuilder()
                          .setPath(rootDirectory.resolve(fi.getInfo().getPath()).toString());
                      return builder.build();
                    })
                .collect(Collectors.toList()));
    this.fileDataProvider = fileDataProvider;
  }

  public static CompilationUnitFileSystem create(
      CompilationUnit compilationUnit, FileDataProvider fileDataProvider) {
    return new CompilationUnitFileSystem(
        CompilationUnitFileSystemProvider.instance(), compilationUnit, fileDataProvider);
  }

  @Override
  public FileSystemProvider provider() {
    return provider;
  }

  @Override
  public void close() throws IOException {
    synchronized (this) {
      if (closed) return;
      try {
        fileDataProvider.close();
      } catch (IOException e) {
        throw e;
      } catch (Exception e) {
        throw new RuntimeException(e);
      } finally {
        fileDataProvider = null;
        compilationFileTree = null;
        closed = true;
      }
    }
  }

  @Override
  public boolean isOpen() {
    return !closed;
  }

  @Override
  public boolean isReadOnly() {
    return true;
  }

  @Override
  public String getSeparator() {
    return "/";
  }

  @Override
  public Iterable<Path> getRootDirectories() {
    return ImmutableList.of(rootDirectory);
  }

  Path getRootDirectory() {
    return rootDirectory;
  }

  @Override
  public Iterable<FileStore> getFileStores() {
    return ImmutableList.of();
  }

  @Override
  public Set<String> supportedFileAttributeViews() {
    return SUPPORTED_VIEWS;
  }

  @Override
  public Path getPath(String first, String... more) {
    return new CompilationUnitPath(this, first, more);
  }

  @Override
  public PathMatcher getPathMatcher(String syntaxAndPattern) {
    return FileSystems.getDefault().getPathMatcher(syntaxAndPattern);
  }

  @Override
  public UserPrincipalLookupService getUserPrincipalLookupService() {
    throw new UnsupportedOperationException();
  }

  @Override
  public WatchService newWatchService() throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  protected void finalize() throws Throwable {
    try {
      close();
    } finally {
      super.finalize();
    }
  }

  String digest(Path file) throws IOException {
    checkNotNull(file);
    file = getRootDirectory().resolve(file);
    if (file.getFileName() == null) {
      // Special case root because getFileName() on "/" returns null.
      return CompilationUnitFileTree.DIRECTORY_DIGEST;
    }
    Path dir = file.getParent();
    return compilationFileTree.lookup(
        dir == null ? null : dir.toString(), file.getFileName().toString());
  }

  void checkAccess(Path path) throws IOException {
    if (digest(path) == null) {
      throw new FileNotFoundException();
    }
  }

  Iterable<Path> list(Path dir) throws IOException {
    final Path abs = getRootDirectory().resolve(dir);
    Map<String, String> entries = compilationFileTree.list(abs.toString());
    if (entries == null) {
      throw new FileNotFoundException(dir.toString());
    }
    return entries.keySet().stream().map(k -> dir.resolve(k)).collect(Collectors.toSet());
  }

  byte[] read(Path file) throws IOException {
    String digest = digest(file);
    if (digest == null || digest == CompilationUnitFileTree.DIRECTORY_DIGEST) {
      throw new FileNotFoundException(file.toString());
    }
    try {
      return fileDataProvider.startLookup(file.toString(), digest).get();
    } catch (InterruptedException | ExecutionException exc) {
      throw new IOException(exc);
    }
  }

  CompilationUnitFileAttributes readAttributes(Path path) throws IOException {
    String digest = digest(path);
    if (digest == null) {
      throw new FileNotFoundException(path.toString());
    }
    return new CompilationUnitFileAttributes(
        digest == CompilationUnitFileTree.DIRECTORY_DIGEST ? -1 : 1);
  }
}
