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

import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.proto.Analysis;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/** Read a kzip file. */
public final class KZipReader implements KZip.Reader {

  private final Gson gson;
  private final ZipFile zipFile;
  private final KZip.Descriptor descriptor;

  public KZipReader(File file) throws IOException {
    this(file, KZip.buildGson(new GsonBuilder()));
  }

  public KZipReader(File file, GsonBuilder gsonBuilder) throws IOException {
    this(file, KZip.buildGson(gsonBuilder));
  }

  public KZipReader(File file, Gson gson) throws IOException {
    this.gson = gson;
    this.zipFile = new ZipFile(file, ZipFile.OPEN_READ);
    this.descriptor = getDescriptor(this.zipFile);
  }

  private static KZip.Descriptor getDescriptor(ZipFile zipFile) {
    Enumeration<? extends ZipEntry> zipEntries = zipFile.entries();
    if (!zipEntries.hasMoreElements()) {
      throw new KZipException("missing root entry");
    }
    ZipEntry root = zipEntries.nextElement();
    if (!root.isDirectory()) {
      throw new KZipException("invalid root entry: " + root.getName());
    }
    String rootPrefix = root.getName();

    Set<String> jsonUnits = new HashSet<>();
    Set<String> protoUnits = new HashSet<>();
    String jsonPrefix = rootPrefix + KZip.Encoding.JSON.getSubdirectory();
    String protoPrefix = rootPrefix + KZip.Encoding.PROTO.getSubdirectory();

    // Make sure each path in the kzip has the same root.
    // Also accumulate potential unit digests.
    while (zipEntries.hasMoreElements()) {
      ZipEntry zipEntry = zipEntries.nextElement();
      String name = zipEntry.getName();
      if (!name.startsWith(rootPrefix)) {
        throw new KZipException("Invalid entry (bad root): " + name);
      }
      if (name.startsWith(jsonPrefix)) {
	jsonUnits.add(name.substring(jsonPrefix.length() + 1));
      }
      if (name.startsWith(protoPrefix)) {
	protoUnits.add(name.substring(protoPrefix.length() + 1));
      }
    }
    KZip.Encoding encoding = KZip.Encoding.PROTO;
    if (jsonUnits.isEmpty()) {
      if (protoUnits.isEmpty()) {
	throw new KZipException("kzip contains no compilation units");
      }
    } else if (protoUnits.isEmpty()) {
      encoding = KZip.Encoding.JSON;
    } else if (!jsonUnits.equals(protoUnits)) {
      throw new KZipException("KZip has proto and json encoded units but they are not equal");
    }
    return KZip.Descriptor.create(rootPrefix, encoding);
  }

  @Override
  public Iterable<Analysis.IndexedCompilation> scan() {
    String unitPrefix = descriptor.getUnitsPath("/");
    return () ->
        zipFile.stream()
            .filter(entry -> !entry.isDirectory())
            .filter(entry -> entry.getName().startsWith(unitPrefix))
            .map(this::readUnit)
            .iterator();
  }

  @Override
  public Analysis.IndexedCompilation readUnit(String unitDigest) {
    return readUnit(zipFile.getEntry(descriptor.getUnitsPath(unitDigest)));
  }

  private Analysis.IndexedCompilation readUnit(ZipEntry entry) {
    try (InputStream input = zipFile.getInputStream(entry)) {
      if (descriptor.encoding() == KZip.Encoding.JSON) {
        try (InputStreamReader reader = new InputStreamReader(input, KZip.DATA_CHARSET)) {
	  return gson.fromJson(reader, Analysis.IndexedCompilation.class);
	} 
      }
      return Analysis.IndexedCompilation.parseFrom(input);
    } catch (IOException e) {
      throw new KZipException("Unable to read unit: " + entry.getName(), e);
    }
  }

  @Override
  public byte[] readFile(String fileDigest) {
    return readFile(zipFile.getEntry(descriptor.getFilesPath(fileDigest)));
  }

  private byte[] readFile(ZipEntry entry) {
    try (InputStream input = zipFile.getInputStream(entry)) {
      return ByteStreams.toByteArray(input);
    } catch (IOException e) {
      throw new KZipException("Unable to read file: " + entry.getName(), e);
    }
  }
}
