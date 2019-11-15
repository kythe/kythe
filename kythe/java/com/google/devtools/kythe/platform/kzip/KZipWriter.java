/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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
package com.google.devtools.kythe.platform.kzip;

import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.platform.shared.CompilationUnits;
import com.google.devtools.kythe.proto.Analysis;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/** Write a kzip file. */
public final class KZipWriter implements KZip.Writer {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  // Constant time used so time information is not stored in the kzip. This way files can be diff'd
  // and they will only be different if the contents are different.
  private static final long MODIFIED_TIME = 0;
  private static final KZip.Encoding DEFAULT_ENCODING;

  static {
    KZip.Encoding encoding = KZip.Encoding.JSON;
    String encodingStr = System.getenv("KYTHE_KZIP_ENCODING");
    if (encodingStr != null) {
      try {
        encoding = KZip.Encoding.valueOf(encodingStr.toUpperCase());
      } catch (IllegalArgumentException e) {
        logger.atWarning().log("Unknown kzip encoding '%s', using %s", encodingStr, encoding);
      }
    }
    DEFAULT_ENCODING = encoding;
  }

  private final KZip.Descriptor descriptor;
  private final ZipOutputStream output;
  private final Gson gson;

  // We store our own set of paths written, analogous to ZipOutputWriter.names, so that we avoid
  // raising exceptions by writing duplicates.
  private final Set<String> pathsWritten;

  @Deprecated
  public KZipWriter(File file) throws IOException {
    this(file, DEFAULT_ENCODING);
  }

  public KZipWriter(File file, KZip.Encoding encoding) throws IOException {
    this(file, encoding, KZip.buildGson(new GsonBuilder()));
  }

  public KZipWriter(File file, KZip.Encoding encoding, GsonBuilder gsonBuilder) throws IOException {
    this(file, encoding, KZip.buildGson(gsonBuilder));
  }

  public KZipWriter(File file, KZip.Encoding encoding, Gson gson) throws IOException {
    FileOutputStream out = new FileOutputStream(file);
    this.output = new ZipOutputStream(out);
    this.gson = gson;
    this.descriptor = KZip.Descriptor.create("root", encoding);
    // Add an entry for the root directory prefix (required by the spec).
    ZipEntry root = new ZipEntry(descriptor.root() + "/");
    root.setComment("kzip root directory");
    root.setTime(MODIFIED_TIME);
    this.output.putNextEntry(root);
    this.output.closeEntry();

    this.pathsWritten = new HashSet<>();
  }

  @Override
  public String writeUnit(Analysis.IndexedCompilation compilation) throws IOException {
    String digest = CompilationUnits.digestFor(compilation.getUnit());
    if (descriptor.encoding().equals(KZip.Encoding.JSON)
        || descriptor.encoding().equals(KZip.Encoding.ALL)) {
      byte[] jsonData =
          gson.toJson(compilation, Analysis.IndexedCompilation.class).getBytes(KZip.DATA_CHARSET);
      appendZip(jsonData, descriptor.getUnitsPath(digest, KZip.Encoding.JSON));
    }
    if (descriptor.encoding().equals(KZip.Encoding.PROTO)
        || descriptor.encoding().equals(KZip.Encoding.ALL)) {
      appendZip(compilation.toByteArray(), descriptor.getUnitsPath(digest, KZip.Encoding.PROTO));
    }
    return digest;
  }

  @Override
  public String writeFile(String data) throws IOException {
    return writeFile(data.getBytes(KZip.DATA_CHARSET));
  }

  @Override
  public String writeFile(byte[] data) throws IOException {
    String digest = KZip.DATA_DIGEST.hashBytes(data).toString();
    String filePath = descriptor.getFilesPath(digest);
    appendZip(data, filePath);
    return digest;
  }

  /**
   * Add a file in the zip file.
   *
   * @param data contents of file to add
   * @param path path in the zip file to create
   */
  private void appendZip(byte[] data, String path) throws IOException {
    if (pathsWritten.add(path)) {
      ZipEntry entry = new ZipEntry(path);
      entry.setTime(MODIFIED_TIME);
      output.putNextEntry(entry);
      output.write(data);
      output.closeEntry();
    } else {
      logger.atWarning().log("Warning: Already wrote %s to kzip.", path);
    }
  }

  @Override
  public void close() throws IOException {
    output.close();
  }
}
