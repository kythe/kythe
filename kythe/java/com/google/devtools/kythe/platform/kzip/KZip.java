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

import com.google.auto.value.AutoValue;
import com.google.common.hash.HashFunction;
import com.google.common.hash.Hashing;
import com.google.devtools.kythe.proto.Analysis;
import com.google.devtools.kythe.proto.Analysis.IndexedCompilation;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Paths;

/**
 * Handles reading from and writing to files in the kzip format, a standard format for storage of
 * Kythe compilation records and their required inputs.
 *
 * <p>The file format is documented at http://www.kythe.io/docs/kythe-kzip.html. The basic format is
 * a zip file that looks like this:
 *
 * <pre>
 *   root/
 *     units/
 *       DF87A837
 *     files/
 *       71908ABB
 *       9380138B
 * </pre>
 *
 * where each file name is the hash of its contents. The files under "units" are json-encoded protos
 * and the files under "files" raw bytes (some may be source text files, others may be compilation
 * artifacts).
 *
 * <p>If you will be reading or writing messages that contain an Any field, you will need to
 * register those types for JSON. You will need to do something like this in your code:
 *
 * <pre>
 *   private static final JsonFormat.TypeRegistry JSON_TYPE_REGISTRY =
 *     JsonFormat.TypeRegistry.newBuilder()
 *     .add(Go.GoDetails.getDescriptor())
 *     .build();
 *
 *   public void setup() { JsonUtil.usingTypeRegistry(JSON_TYPE_REGISTRY); }
 * </pre>
 *
 * <p>You will then need to call setup somewhere in your code.
 */
public class KZip {

  /** Read from a KZip. */
  public interface Reader {

    /** Returns all the compilation records in the kzip, in unspecified order. */
    Iterable<IndexedCompilation> scan();

    /**
     * Returns the compilation record corresponding to the given unit digest. Throws a {@link
     * KZipException} if the unit cannot be read.
     */
    IndexedCompilation readUnit(String unitDigest);

    /**
     * Reads and returns the requested file data. Throws a {@link KZipException} if the file cannot
     * be read.
     */
    byte[] readFile(String fileDigest);
  }

  /** Write to a KZip. */
  public interface Writer extends AutoCloseable {

    /** Adds the specified compilation to the archive, and returns the digest of the stored unit. */
    String writeUnit(Analysis.IndexedCompilation unit) throws IOException;

    /**
     * Adds the specified file contents to the archive, and returns the digest of the stored data.
     */
    String writeFile(byte[] data) throws IOException;

    /**
     * Adds the specified file contents to the archive, and returns the digest of the stored data.
     */
    String writeFile(String data) throws IOException;

    /** Flushes and closes the writer, and its underlying stream */
    @Override
    void close() throws IOException;
  }

  static final Charset DATA_CHARSET = StandardCharsets.UTF_8;
  static final HashFunction DATA_DIGEST = Hashing.sha256();
  private static final String FILES_SUBDIRECTORY = "files";

  private KZip() {}

  // TODO(salguarnieri) See if we can avoid using Gson entirely and use the json support in
  // GeneratedMessageV3.

  static Gson buildGson(GsonBuilder builder) {
    return JsonUtil.registerProtoTypes(builder).create();
  }

  public static enum Encoding {
    JSON("units"),
    PROTO("pbunits"),
    ALL("");

    private Encoding(String subdirectory) {
      this.subdirectory = subdirectory;
    }

    String getSubdirectory() {
      return subdirectory;
    }

    private final String subdirectory;
  }

  @AutoValue
  abstract static class Descriptor {
    abstract String root();

    abstract Encoding encoding();

    static Descriptor create(String root, Encoding encoding) {
      return new AutoValue_KZip_Descriptor(root, encoding);
    }

    String getUnitsPath(String digest) {
      return getUnitsPath(digest, encoding());
    }

    String getUnitsPath(String digest, Encoding encoding) {
      return Paths.get(root(), encoding.getSubdirectory(), digest).toString();
    }

    String getFilesPath(String digest) {
      return Paths.get(root(), FILES_SUBDIRECTORY, digest).toString();
    }
  }
}
