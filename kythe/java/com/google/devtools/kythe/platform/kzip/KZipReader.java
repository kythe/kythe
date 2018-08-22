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

import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.proto.Analysis;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/** Read a kzip file. */
public final class KZipReader implements KZip.Reader {

  private final Gson gson;
  private final ZipFile zipFile;
  private final String rootPrefix;

  public KZipReader(File file) throws IOException {
    this(file, KZip.buildGson(new GsonBuilder()));
  }

  public KZipReader(File file, GsonBuilder gsonBuilder) throws IOException {
    this(file, KZip.buildGson(gsonBuilder));
  }

  public KZipReader(File file, Gson gson) throws IOException {
    this.gson = gson;
    this.zipFile = new ZipFile(file, ZipFile.OPEN_READ);
    this.rootPrefix = getRootPrefix(this.zipFile);
  }

  private static String getRootPrefix(ZipFile zipFile) {
    Enumeration<? extends ZipEntry> zipEntires = zipFile.entries();
    if (!zipEntires.hasMoreElements()) {
      throw new KZipException("missing root entry");
    }
    ZipEntry root = zipEntires.nextElement();
    if (!root.isDirectory()) {
      throw new KZipException("invalid root entry: " + root.getName());
    }
    String rootPrefix = root.getName();

    // Make sure each path in the kzip has the same root.
    while (zipEntires.hasMoreElements()) {
      ZipEntry zipEntry = zipEntires.nextElement();
      if (!zipEntry.getName().startsWith(rootPrefix)) {
        throw new KZipException("Invalid entry (bad root): " + zipEntry.getName());
      }
    }
    return rootPrefix;
  }

  @Override
  public Iterator<Analysis.IndexedCompilation> scan() {
    String unitPrefix = KZip.getUnitsPath(rootPrefix, "");
    return zipFile
        .stream()
        .filter(entry -> entry.getName().startsWith(unitPrefix))
        .map(this::readUnit)
        .iterator();
  }

  @Override
  public Analysis.IndexedCompilation readUnit(String unitDigest) {
    return readUnit(zipFile.getEntry(KZip.getUnitsPath(rootPrefix, unitDigest)));
  }

  private Analysis.IndexedCompilation readUnit(ZipEntry entry) {
    try (InputStream input = zipFile.getInputStream(entry);
        InputStreamReader reader = new InputStreamReader(input, KZip.DATA_CHARSET)) {
      return gson.fromJson(reader, Analysis.IndexedCompilation.class);
    } catch (IOException e) {
      throw new KZipException("Unable to read unit: " + entry.getName(), e);
    }
  }

  @Override
  public byte[] readFile(String fileDigest) {
    return readFile(zipFile.getEntry(KZip.getFilesPath(rootPrefix, fileDigest)));
  }

  private byte[] readFile(ZipEntry entry) {
    try (InputStream input = zipFile.getInputStream(entry)) {
      return ByteStreams.toByteArray(input);
    } catch (IOException e) {
      throw new KZipException("Unable to read file: " + entry.getName(), e);
    }
  }
}
