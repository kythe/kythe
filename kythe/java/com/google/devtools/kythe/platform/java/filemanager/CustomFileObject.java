/*
 * Copyright 2014 Google Inc. All rights reserved.
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

import com.google.devtools.kythe.platform.shared.FileDataProvider;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Reader;
import java.io.Writer;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Objects;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import javax.tools.FileObject;

/** File object backed by a {@link Future}. */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public class CustomFileObject implements FileObject {
  protected final Path path;
  protected final String digest;
  protected final Future<byte[]> future;

  protected final Charset encoding;

  public CustomFileObject(
      FileDataProvider contentProvider, String path, String digest, Charset encoding) {
    this.path = Paths.get(path);
    this.digest = digest;
    this.encoding = encoding;

    this.future = contentProvider.startLookup(path, digest);
  }

  @Override
  public URI toUri() {
    try {
      return new URI("custom://" + digest + "/" + path);
    } catch (URISyntaxException e) {
      return null;
    }
  }

  @Override
  public String getName() {
    return path.getFileName().toString();
  }

  @Override
  public InputStream openInputStream() throws IOException {
    return new ByteArrayInputStream(getData());
  }

  private byte[] getData() throws IOException {
    try {
      byte[] result = future.get();
      if (result == null) {
        throw new IOException(
            String.format("Unable to find file with digest %s, path %s", digest, path));
      }
      return result;
    } catch (InterruptedException e) {
      throw new IOException(e);
    } catch (ExecutionException e) {
      throw new IOException(e);
    }
  }

  @Override
  public OutputStream openOutputStream() {
    throw new UnsupportedOperationException();
  }

  @Override
  public Reader openReader(boolean ignoreEncodingErrors) throws IOException {
    return new InputStreamReader(openInputStream());
  }

  @Override
  public CharSequence getCharContent(boolean ignoreEncodingErrors) throws IOException {
    return new String(getData(), encoding);
  }

  @Override
  public Writer openWriter() {
    throw new UnsupportedOperationException();
  }

  @Override
  public long getLastModified() {
    return 0;
  }

  @Override
  public boolean delete() {
    return false;
  }

  @Override
  public int hashCode() {
    return Objects.hashCode(digest);
  }

  @Override
  public boolean equals(Object other) {
    // This implementation relies on the Trusted annotation on this class to opt out of using the
    // WrappedJavaFileObject wrapper class.
    if (this == other) {
      return true;
    } else if (!(other instanceof CustomFileObject)) {
      return false;
    }
    CustomFileObject oc = (CustomFileObject) other;
    return Objects.equals(path, oc.path) && Objects.equals(digest, oc.digest);
  }
}
