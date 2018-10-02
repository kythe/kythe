/*
 * Copyright 2018 Google Inc. All rights reserved.
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

import com.google.devtools.kythe.proto.Analysis;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipOutputStream;

/** Write a kzip file. */
public final class KZipWriter implements KZip.Writer {

  private static final String ROOT_PREFIX = "root/";
  private final ZipOutputStream output;
  private final Gson gson;

  public KZipWriter(File file) throws IOException {
    this(file, KZip.buildGson(new GsonBuilder()));
  }

  public KZipWriter(File file, GsonBuilder gsonBuilder) throws IOException {
    this(file, KZip.buildGson(gsonBuilder));
  }

  public KZipWriter(File file, Gson gson) throws IOException {
    FileOutputStream out = new FileOutputStream(file);
    this.output = new ZipOutputStream(out);
    this.gson = gson;

    // Add an entry for the root directory prefix (required by the spec).
    ZipEntry root = new ZipEntry(ROOT_PREFIX);
    root.setComment("kzip root directory");
    this.output.putNextEntry(root);
    this.output.closeEntry();
  }

  @Override
  public String writeUnit(Analysis.IndexedCompilation compilation) throws IOException {
    byte[] jsonData =
        gson.toJson(compilation, Analysis.IndexedCompilation.class).getBytes(KZip.DATA_CHARSET);
    String digest = KZip.DATA_DIGEST.hashBytes(jsonData).toString();
    String compilationPath = KZip.getUnitsPath(ROOT_PREFIX, digest);
    appendZip(jsonData, compilationPath);
    return digest;
  }

  @Override
  public String writeFile(String data) throws IOException {
    return writeFile(data.getBytes(KZip.DATA_CHARSET));
  }

  @Override
  public String writeFile(byte[] data) throws IOException {
    String digest = KZip.DATA_DIGEST.hashBytes(data).toString();
    String filePath = KZip.getFilesPath(ROOT_PREFIX, digest);
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
    ZipEntry entry = new ZipEntry(path);
    try {
      output.putNextEntry(entry);
    } catch (ZipException e) {
      // For the specific case of duplicates, we want to just safely continue
      // (return without writing).
      if (e.getMessage().startsWith("duplicate entry: ")) {
        return;
      }
      throw e;
    }
    output.write(data);
    output.closeEntry();
  }

  @Override
  public void close() throws IOException {
    output.close();
  }
}
