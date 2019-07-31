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

import static com.google.common.base.Preconditions.checkState;

import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.DirectoryStream;
import java.nio.file.FileSystem;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.WatchEvent;
import java.nio.file.WatchKey;
import java.nio.file.WatchService;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/** A Path for CompilationUnit-based paths. */
final class CompilationUnitPath implements Path {
  private final CompilationUnitFileSystem fileSystem;
  private final Path path; // Use default file system for slash-delimited path manipulation.

  CompilationUnitPath(CompilationUnitFileSystem fileSystem, String path, String... more) {
    this(fileSystem, Paths.get(path, more));
  }

  private CompilationUnitPath(CompilationUnitFileSystem fileSystem, Path path) {
    this.fileSystem = fileSystem;
    this.path = path;
  }

  @Override
  public FileSystem getFileSystem() {
    return fileSystem;
  }

  @Override
  public boolean isAbsolute() {
    return path.isAbsolute();
  }

  @Override
  public Path getRoot() {
    return isAbsolute() ? fileSystem.getRootDirectory() : null;
  }

  @Override
  public Path getFileName() {
    return wrap(path.getFileName());
  }

  @Override
  public Path getParent() {
    return wrap(path.getParent());
  }

  @Override
  public int getNameCount() {
    return path.getNameCount();
  }

  @Override
  public Path getName(int index) {
    return wrap(path.getName(index));
  }

  @Override
  public Path subpath(int beginIndex, int endIndex) {
    return wrap(path.subpath(beginIndex, endIndex));
  }

  @Override
  public boolean startsWith(Path other) {
    return getFileSystem() == other.getFileSystem() && path.startsWith(unwrap(other));
  }

  @Override
  public boolean startsWith(String other) {
    return path.startsWith(other);
  }

  @Override
  public boolean endsWith(Path other) {
    return getFileSystem() == other.getFileSystem() && path.endsWith(unwrap(other));
  }

  @Override
  public boolean endsWith(String other) {
    return path.endsWith(other);
  }

  @Override
  public Path normalize() {
    return wrap(path.normalize());
  }

  @Override
  public Path resolve(Path other) {
    if (other.isAbsolute()) return other;
    if (other.toString().isEmpty()) return this;
    return wrap(path.resolve(unwrap(other)));
  }

  @Override
  public Path resolve(String other) {
    return other.isEmpty() ? this : wrap(path.resolve(other));
  }

  @Override
  public Path resolveSibling(Path other) {
    return wrap(path.resolveSibling(unwrap(other)));
  }

  @Override
  public Path resolveSibling(String other) {
    return wrap(path.resolveSibling(other));
  }

  @Override
  public Path relativize(Path other) {
    return wrap(path.relativize(unwrap(other)));
  }

  @Override
  public URI toUri() {
    checkState(isAbsolute(), "Absolute path required.");
    try {
      return new URI(
          fileSystem.provider().getScheme(),
          /* host= */ null,
          /* path= */ path.toString(),
          /* fragment= */ null);
    } catch (URISyntaxException ex) {
      throw new AssertionError(ex);
    }
  }

  @Override
  public Path toAbsolutePath() {
    return isAbsolute() ? this : null;
  }

  @Override
  public Path toRealPath(LinkOption... options) throws IOException {
    return toAbsolutePath().normalize();
  }

  @Override
  public File toFile() {
    throw new UnsupportedOperationException();
  }

  @Override
  public WatchKey register(
      WatchService watcher, WatchEvent.Kind<?>[] events, WatchEvent.Modifier... modifiers)
      throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public WatchKey register(WatchService watcher, WatchEvent.Kind<?>... events) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public Iterator<Path> iterator() {
    return new Iterator<Path>() {
      private final Iterator<Path> inner = path.iterator();

      @Override
      public boolean hasNext() {
        return inner.hasNext();
      }

      @Override
      public Path next() {
        return wrap(inner.next());
      }
    };
  }

  @Override
  public int compareTo(Path other) {
    return path.compareTo(unwrap(other));
  }

  @Override
  public boolean equals(Object other) {
    return other instanceof CompilationUnitPath
        && fileSystem == ((CompilationUnitPath) other).fileSystem
        && path.equals(((CompilationUnitPath) other).path);
  }

  @Override
  public int hashCode() {
    return path.hashCode();
  }

  @Override
  public String toString() {
    return path.toString();
  }

  void checkAccess() throws IOException {
    fileSystem.checkAccess(this);
  }

  boolean isSameFile(Path other) {
    return equals(other);
  }

  CompilationUnitFileAttributes readAttributes() throws IOException {
    return fileSystem.readAttributes(this);
  }

  SeekableByteChannel newByteChannel() throws IOException {
    return new ByteBufferByteChannel(fileSystem.read(this));
  }

  DirectoryStream<Path> newDirectoryStream(DirectoryStream.Filter<? super Path> filter)
      throws IOException {
    final Iterable<Path> entries = fileSystem.list(this);
    final List<Path> filtered = new ArrayList<>();
    for (Path p : entries) {
      if (filter.accept(p)) {
        filtered.add(p);
      }
    }
    return new DirectoryStream<Path>() {
      @Override
      public Iterator<Path> iterator() {
        return filtered.iterator();
      }

      @Override
      public void close() throws IOException {}
    };
  }

  private CompilationUnitPath wrap(Path path) {
    return path == null ? null : new CompilationUnitPath(fileSystem, path);
  }

  private static Path unwrap(Path path) {
    if (path instanceof CompilationUnitPath) {
      return ((CompilationUnitPath) path).path;
    }
    return path;
  }
}
