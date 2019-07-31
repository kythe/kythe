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

import java.io.IOException;
import java.net.URI;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.AccessMode;
import java.nio.file.CopyOption;
import java.nio.file.DirectoryStream;
import java.nio.file.FileStore;
import java.nio.file.FileSystem;
import java.nio.file.LinkOption;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.ProviderMismatchException;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.FileAttributeView;
import java.nio.file.spi.FileSystemProvider;
import java.util.Map;
import java.util.Set;

/** {@link FileSystemProvider} for a read-only CompilationUnit-based filesystem. */
final class CompilationUnitFileSystemProvider extends FileSystemProvider {
  private static final CompilationUnitFileSystemProvider INSTANCE =
      new CompilationUnitFileSystemProvider();
  private static final String SCHEME = "x-kythe-compilation";

  // TODO(shahms): register a URL stream handler.

  /** Returns the singleton instance of this provider. */
  static CompilationUnitFileSystemProvider instance() {
    return INSTANCE;
  }

  @Override
  public String getScheme() {
    return SCHEME;
  }

  @Override
  public FileSystem newFileSystem(URI uri, Map<String, ?> env) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public FileSystem getFileSystem(URI uri) {
    throw new UnsupportedOperationException();
  }

  @Override
  public Path getPath(URI uri) {
    throw new UnsupportedOperationException();
  }

  @Override
  public SeekableByteChannel newByteChannel(
      Path path, Set<? extends OpenOption> options, FileAttribute<?>... attrs) throws IOException {
    // TODO(shahms): Check options and attrs rather than just ignoring them?
    return toCompilationUnitPath(path).newByteChannel();
  }

  @Override
  public DirectoryStream<Path> newDirectoryStream(
      Path dir, DirectoryStream.Filter<? super Path> filter) throws IOException {
    return toCompilationUnitPath(dir).newDirectoryStream(filter);
  }

  @Override
  public void createDirectory(Path dir, FileAttribute<?>... attrs) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void delete(Path path) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void copy(Path source, Path target, CopyOption... options) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void move(Path source, Path target, CopyOption... options) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public boolean isSameFile(Path a, Path b) throws IOException {
    return toCompilationUnitPath(a).isSameFile(b);
  }

  @Override
  public boolean isHidden(Path path) throws IOException {
    return false;
  }

  @Override
  public FileStore getFileStore(Path path) throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void checkAccess(Path path, AccessMode... modes) throws IOException {
    for (AccessMode mode : modes) {
      if (mode != AccessMode.READ) {
        throw new IOException("read-only filesystem");
      }
    }
    toCompilationUnitPath(path).checkAccess();
  }

  @Override
  public <V extends FileAttributeView> V getFileAttributeView(
      Path path, Class<V> type, LinkOption... options) {
    throw new UnsupportedOperationException();
  }

  @Override
  public <A extends BasicFileAttributes> A readAttributes(
      Path path, Class<A> type, LinkOption... options) throws IOException {
    if (!type.isAssignableFrom(CompilationUnitFileAttributes.class)) {
      throw new UnsupportedOperationException(String.format("%s is not supported", type));
    }
    return type.cast(toCompilationUnitPath(path).readAttributes());
  }

  @Override
  public Map<String, Object> readAttributes(Path path, String attributes, LinkOption... options)
      throws IOException {
    throw new UnsupportedOperationException();
  }

  @Override
  public void setAttribute(Path path, String attribute, Object value, LinkOption... options)
      throws IOException {
    throw new UnsupportedOperationException();
  }

  private static CompilationUnitPath toCompilationUnitPath(Path path)
      throws ProviderMismatchException {
    checkNotNull(path);
    if (!(path instanceof CompilationUnitPath)) {
      throw new ProviderMismatchException();
    }
    return (CompilationUnitPath) path;
  }
}
