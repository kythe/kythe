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

package com.google.devtools.kythe.platform.indexpack;

import com.google.common.base.Function;
import com.google.common.base.Preconditions;
import com.google.common.base.Predicate;
import com.google.common.collect.Iterables;
import com.google.common.collect.Iterators;
import com.google.common.hash.HashFunction;
import com.google.common.hash.Hashing;
import com.google.common.io.ByteStreams;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileData;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.google.protobuf.ByteString;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Reader;
import java.lang.reflect.Type;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Iterator;
import java.util.UUID;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

/**
 * Collection of compilation units stored as an index pack directory. The index pack format is
 * defined in kythe/docs/kythe-index-pack.txt.
 */
public class Archive {
  /** {@link Charset} to encode {@link String} file contents. */
  public static final Charset DATA_CHARSET = StandardCharsets.UTF_8;
  /** Format key with which to write {@link CompilationUnit}s. */
  public static final String KYTHE_FORMAT_KEY = "kythe";

  private static final Gson DEFAULT_GSON = buildGson(new GsonBuilder());
  private static final HashFunction DATA_DIGEST = Hashing.sha256();

  private static final Path DATA_DIR = Paths.get("files");
  private static final Path UNIT_DIR = Paths.get("units");
  private static final String DATA_SUFFIX = ".data";
  private static final String UNIT_SUFFIX = ".unit";
  private static final String NEW_SUFFIX = ".new";

  private final Gson gson;
  private final Path rootDir, dataDir, unitDir;

  /** Opens an existing index pack, if one exists; or if not, then attempts to create one. */
  public Archive(String path) {
    this(Paths.get(path));
  }

  /** Opens an existing index pack, if one exists; or if not, then attempts to create one. */
  public Archive(Path path) {
    this(path, DEFAULT_GSON);
  }

  /**
   * Opens an existing index pack, if one exists; or if not, then attempts to create one. Uses
   * {@code builder} as the base {@link Gson} object for serializing compilation units. Type
   * adapters will be registered for the index pack's internal JSON structures as well as protobuf
   * types.
   */
  public Archive(Path path, GsonBuilder builder) {
    this(path, buildGson(builder));
  }

  private Archive(Path path, Gson gson) {
    this.gson = gson;
    rootDir = path;
    dataDir = path.resolve(DATA_DIR);
    unitDir = path.resolve(UNIT_DIR);

    File dataDirFile = dataDir.toFile();
    dataDirFile.mkdirs();
    if (!dataDirFile.isDirectory()) {
      throw new IllegalArgumentException(dataDir + " is not a directory");
    }
    File unitDirFile = unitDir.toFile();
    unitDirFile.mkdirs();
    if (!unitDirFile.isDirectory()) {
      throw new IllegalArgumentException(unitDir + " is not a directory");
    }
  }

  /** Returns the index pack's root directory. */
  public Path getRoot() {
    return rootDir;
  }

  /**
   * Returns an {@link Iterator} of the {@link CompilationUnit}s stored in the index pack along with
   * each unit's required inputs.
   */
  public Iterator<CompilationDescription> readDescriptions() throws IOException {
    return Iterators.transform(
        readUnits(),
        new Function<CompilationUnit, CompilationDescription>() {
          @Override
          public CompilationDescription apply(CompilationUnit unit) {
            return completeDescription(unit);
          }
        });
  }

  /** Returns a complete {@link CompilationDescription} of the given {@link CompilationUnit}. */
  public CompilationDescription completeDescription(CompilationUnit unit) {
    return new CompilationDescription(
        unit,
        Iterables.transform(
            unit.getRequiredInputList(),
            new Function<FileInput, FileData>() {
              @Override
              public FileData apply(FileInput input) {
                try {
                  return FileData.newBuilder()
                      .setInfo(input.getInfo())
                      .setContent(ByteString.copyFrom(readFile(input.getInfo().getDigest())))
                      .build();
                } catch (IOException ioe) {
                  throw new RuntimeException(ioe);
                }
              }
            }));
  }

  /**
   * Returns a complete {@link CompilationDescription} for the {@link CompilationUnit} stored in the
   * index pack with the given digest key.
   */
  public CompilationDescription readDescription(String key) throws IOException {
    return completeDescription(readUnit(key));
  }

  /** Returns an {@link Iterator} of the {@link CompilationUnit}s stored in the archive. */
  public Iterator<CompilationUnit> readUnits() throws IOException {
    return readUnits(KYTHE_FORMAT_KEY, CompilationUnit.class);
  }

  /** Returns the {@link CompilationUnit} with the given digest key. */
  public CompilationUnit readUnit(String key) throws IOException {
    CompilationUnit unit = readUnit(key, KYTHE_FORMAT_KEY, CompilationUnit.class);
    if (unit == null) {
      throw new IllegalArgumentException("Unit with key '" + key + "' is not a CompilationUnit");
    }
    return unit;
  }

  /** Returns an {@link Iterator} of the units stored in the archive with a given format key. */
  public <T> Iterator<T> readUnits(final String formatKey, final Class<T> cls) throws IOException {
    Preconditions.checkNotNull(formatKey);
    return Iterators.filter(
        Iterators.transform(
            Files.newDirectoryStream(unitDir, "*" + UNIT_SUFFIX).iterator(),
            new Function<Path, T>() {
              @Override
              public T apply(Path path) {
                try {
                  String name = path.getFileName().toString();
                  if (!name.endsWith(UNIT_SUFFIX)) {
                    throw new IllegalStateException("Received path without unit suffix: " + path);
                  }
                  String key = name.substring(0, name.length() - UNIT_SUFFIX.length());
                  return readUnit(key, formatKey, cls);
                } catch (IOException ioe) {
                  throw new RuntimeException(ioe);
                }
              }
            }),
        new Predicate<T>() {
          @Override
          public boolean apply(T unit) {
            return unit != null;
          }
        });
  }

  /**
   * Returns the unit with given digest key, checking that it has the given format key. If the unit
   * has a different key, {@code null} is returned.
   */
  public <T> T readUnit(String digestKey, String formatKey, Class<T> cls) throws IOException {
    Path path = unitDir.resolve(digestKey + UNIT_SUFFIX);
    try (InputStream raw = Files.newInputStream(path);
        InputStream uncompressed = new GZIPInputStream(raw);
        Reader reader = new InputStreamReader(uncompressed, DATA_CHARSET)) {
      UnitWrapper wrapper = gson.fromJson(reader, UnitWrapper.class);
      return formatKey.equals(wrapper.formatKey) ? gson.fromJson(wrapper.content, cls) : null;
    }
  }

  /**
   * Returns the contents of the file with the given digest key.
   *
   * @throws java.io.FileNotFoundException if the given file does not exist
   */
  public byte[] readFile(String key) throws IOException {
    return ByteStreams.toByteArray(
        new GZIPInputStream(new FileInputStream(dataDir.resolve(key + DATA_SUFFIX).toFile())));
  }

  /** Writes a {@link CompilationUnit} along with all of its required inputs to the archive. */
  public String writeDescription(CompilationDescription description) throws IOException {
    for (FileData data : description.getFileContents()) {
      writeFile(data.getContent().toByteArray());
    }
    return writeUnit(description.getCompilationUnit());
  }

  /** Writes a {@link CompilationUnit} to the archive using the {@link KYTHE_FORMAT_KEY}. */
  public String writeUnit(CompilationUnit unit) throws IOException {
    return writeUnit(KYTHE_FORMAT_KEY, unit);
  }

  /** Writes a compilation unit to the archive with the given format key. */
  public String writeUnit(String formatKey, Object unit) throws IOException {
    return writeData(
        unitDir, UNIT_SUFFIX, gson.toJson(new UnitWrapper(formatKey, gson.toJsonTree(unit))));
  }

  /** Writes a given file's contents to the {@link Archive} as {@link DATA_CHARSET}. */
  public String writeFile(String data) throws IOException {
    return writeFile(data.getBytes(DATA_CHARSET));
  }

  /** Writes a given file's contents to the {@link Archive}. */
  public String writeFile(byte[] data) throws IOException {
    return writeData(dataDir, DATA_SUFFIX, data);
  }

  private static String writeData(Path dir, String suffix, String data) throws IOException {
    return writeData(dir, suffix, data.getBytes(DATA_CHARSET));
  }

  private static String writeData(Path dir, String suffix, byte[] data) throws IOException {
    File tempFile = dir.resolve(UUID.randomUUID().toString() + NEW_SUFFIX).toFile();
    try (OutputStream intermediary = new FileOutputStream(tempFile);
        OutputStream output = new GZIPOutputStream(intermediary)) {
      output.write(data);
      String key = DATA_DIGEST.hashBytes(data).toString();
      tempFile.renameTo(dir.resolve(key + suffix).toFile());
      return key;
    } finally {
      tempFile.delete();
    }
  }

  private static Gson buildGson(GsonBuilder builder) {
    return JsonUtil.registerProtoTypes(builder)
        .registerTypeAdapter(UnitWrapper.class, new UnitWrapperTypeAdapter())
        .create();
  }

  /** Unit wrapper for JSON encoding/decoding. */
  private static class UnitWrapper {
    final String formatKey;
    final JsonElement content;

    public UnitWrapper(String formatKey, JsonElement unit) {
      this.formatKey = formatKey;
      this.content = unit;
    }
  }

  private static class UnitWrapperTypeAdapter
      implements JsonSerializer<UnitWrapper>, JsonDeserializer<UnitWrapper> {
    private static final String FORMAT_KEY_LABEL = "format";
    private static final String CONTENT_LABEL = "content";

    @Override
    public JsonElement serialize(UnitWrapper unit, Type t, JsonSerializationContext ctx) {
      JsonObject obj = new JsonObject();
      obj.addProperty(FORMAT_KEY_LABEL, unit.formatKey);
      obj.add(CONTENT_LABEL, unit.content);
      return obj;
    }

    @Override
    public UnitWrapper deserialize(JsonElement json, Type t, JsonDeserializationContext ctx) {
      JsonObject obj = json.getAsJsonObject();
      return new UnitWrapper(
          ctx.<String>deserialize(obj.get(FORMAT_KEY_LABEL), String.class), obj.get(CONTENT_LABEL));
    }
  }
}
