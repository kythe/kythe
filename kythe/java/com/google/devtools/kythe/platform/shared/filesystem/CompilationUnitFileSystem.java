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
import com.google.common.util.concurrent.ListenableFuture;
import com.google.devtools.kythe.platform.java.filemanager.CompilationUnitFileTree;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import java.io.IOException;
import java.nio.file.FileStore;
import java.nio.file.FileSystem;
import java.nio.file.FileSystems;
import java.nio.file.NoSuchFileException;
import java.nio.file.NotDirectoryException;
import java.nio.file.Path;
import java.nio.file.PathMatcher;
import java.nio.file.WatchService;
import java.nio.file.attribute.UserPrincipalLookupService;
import java.nio.file.spi.FileSystemProvider;
import java.util.Map;
import java.util.Set;
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
public final class CompilationUnitFileSystem extends FileSystem {

  private static final ImmutableSet<String> SUPPORTED_VIEWS = ImmutableSet.of("basic");

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
  public void close() {
    synchronized (this) {
      if (closed) return;
      fileDataProvider = null;
      compilationFileTree = null;
      closed = true;
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
  public ImmutableList<Path> getRootDirectories() {
    return ImmutableList.of(rootDirectory);
  }

  Path getRootDirectory() {
    return rootDirectory;
  }

  @Override
  public ImmutableList<FileStore> getFileStores() {
    return ImmutableList.of();
  }

  @Override
  public ImmutableSet<String> supportedFileAttributeViews() {
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
  public WatchService newWatchService() {
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

  public ListenableFuture<byte[]> startRead(Path file) throws IOException {
    String digest = digest(file);
    if (digest == null || digest.equals(CompilationUnitFileTree.DIRECTORY_DIGEST)) {
      throw new NoSuchFileException(file.toString());
    }
    return fileDataProvider.startLookup(file.toString(), digest);
  }

  public Set<Path> list(Path dir) throws IOException {
    final Path abs = getRootDirectory().resolve(dir).normalize();
    Map<String, String> entries = compilationFileTree.list(abs.toString());
    if (entries == null) {
      if (digest(abs) == null) {
        throw new NoSuchFileException(dir.toString());
      }
      throw new NotDirectoryException(dir.toString());
    }
    return entries.keySet().stream().map(k -> dir.resolve(k)).collect(Collectors.toSet());
  }

  String digest(Path file) {
    checkNotNull(file);
    file = getRootDirectory().resolve(file).normalize();
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
      throw new NoSuchFileException(path.toString());
    }
  }

  CompilationUnitFileAttributes readAttributes(Path path) throws IOException {
    String digest = digest(path);
    if (digest == null) {
      throw new NoSuchFileException(path.toString());
    }
    return new CompilationUnitFileAttributes(
        digest.equals(CompilationUnitFileTree.DIRECTORY_DIGEST) ? -1 : 1);
  }
}
