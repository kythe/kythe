/*
 * Copyright 2016 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.analyzers.bazel;

import static com.google.common.truth.Truth.assertThat;

import com.google.devtools.build.lib.events.Location;
import com.google.devtools.build.lib.vfs.FileSystem;
import com.google.devtools.build.lib.vfs.JavaIoFileSystem;
import com.google.devtools.kythe.util.KytheURI;
import com.google.devtools.kythe.util.KytheURI.Builder;
import java.io.IOException;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import org.junit.Rule;
import org.junit.Test;
import org.junit.Ignore;
import org.junit.rules.TemporaryFolder;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class TicketsTest {

  @Rule public TemporaryFolder tmpFolder = new TemporaryFolder();

  private final FileSystem fs = new JavaIoFileSystem();

  @Test
  public void fileUri() {
    Tickets tickets = new Tickets(fs, "/kythe/" /* corpusRoot */, "bla/BUILD",
        "werewolf" /* locationUriPrefix */, "swearwolf" /* packageNamePrefix */, "kythe" /* corpus */);
    assertThat(tickets.fileUri("foo/bar/foo.h")).isEqualTo(generateNewKytheUriBuilder(tickets).setPath("/kythe/foo/bar/foo.h").build().toString());
    assertThat(tickets.fileUri(fs.getPath("/kythe/foo/bar.h")))
        .isEqualTo(generateNewKytheUriBuilder(tickets).setPath("/kythe/foo/bar.h").build().toString());
  }

  @Test
  public void relativeToCorpusRoot() {
    Tickets tickets = new Tickets(fs, "/kythe/" /* corpusRoot */, "bla/BUILD",
        "werewolf" /* locationUriPrefix */, "swearwolf" /* packageNamePrefix */, "kythe" /* corpus */);
    assertThat(tickets.relativeToCorpusRoot(fs.getPath("/kythe/foo/bar.h")))
        .isEqualTo("foo/bar.h");
  }

  @Test
  public void tryParseRule() {
    Tickets tickets = new Tickets(fs, "/corpus_root/", "bla/BUILD",
        "werewolf" /* locationUriPrefix */, "swearwolf" /* packageNamePrefix */, "kythe" /* corpus */);
    assertThat(tickets.tryParseRule(":foo")).isEqualTo("build:swearwolf/bla:foo");
    assertThat(tickets.tryParseRule("//foo:bar")).isEqualTo("build:foo:bar");
    assertThat(tickets.tryParseRule("//alice:bob/charlie.eve"))
        .isEqualTo("build:alice:bob/charlie.eve");
    assertThat(tickets.tryParseRule("Foo.java")).isNull();
  }

  @Test
  public void tryParseFilename() throws IOException {
    Path kythe = tmpFolder.newFolder("kythe").toPath();
    Files.createDirectories(kythe.resolve("foo/bar"));
    Files.createFile(kythe.resolve("foo/ExistingFile1.java"));
    Files.createFile(kythe.resolve("foo/bar/ExistingFile2.java"));

    Tickets tickets =
        new Tickets(
            fs,
            kythe.toString(),
            "foo/BUILD",
            "werewolf" /* locationUriPrefix */,
            "swearwolf" /* packageNamePrefix */,
            "kythe" /* corpus */);
    assertThat(tickets.tryParseFilename("NonExistingFile")).isNull();
    assertThat(tickets.tryParseFilename("ExistingFile1.java"))
        .isEqualTo(fs.getPath(String.join(File.separator, kythe.toString(), "foo/ExistingFile1.java")));
    assertThat(tickets.tryParseFilename("bar/ExistingFile2.java"))
        .isEqualTo(fs.getPath(String.join(File.separator, kythe.toString(), "foo/bar/ExistingFile2.java")));
  }

  @Test
  public void realBuildFile() {
    Tickets tickets = new Tickets(fs, "/corpus_root/", "foo/BUILD", "" /* locationUriPrefix */,
        "" /* packageNamePrefix */, "kythe" /* corpus */);
    assertThat(tickets.localRule("Foo")).isEqualTo("build:foo:Foo");
    assertThat(tickets.localRuleDisplayName("Foo")).isEqualTo("foo:Foo");
    assertThat(tickets.positional("Name", Location.fromFileAndOffsets(null, 17, 43)))
        .isEqualTo(String.format("build:Name@%s[17:43]", generateNewKytheUriBuilder(tickets).setPath("/corpus_root/foo/BUILD").build().toString()));
    assertThat(tickets.external("java_library")).isEqualTo("build:java_library");
    assertThat(tickets.getPackageName()).isEqualTo("foo");
    assertThat(tickets.getBasePackageName()).isEqualTo("foo");
  }

  @Test
  public void extensionBuildFile() {
    Tickets tickets = new Tickets(fs, "/corpus_root/", "foo/BUILD", "" /* locationUriPrefix */,
        "EXT" /* packageNamePrefix */, "kythe" /* corpus */);
    assertThat(tickets.localRule("Foo")).isEqualTo("build:EXT/foo:Foo");
    assertThat(tickets.localRuleDisplayName("Foo")).isEqualTo("EXT/foo:Foo");
    // assertThat(tickets.positional("Name", Location.fromFileAndOffsets(null, 17, 43)))
    //     .isEqualTo("build:Name@kythe://kythe/foo/BUILD[17:43]");
    assertThat(tickets.positional("Name", Location.fromFileAndOffsets(null, 17, 43)))
        .isEqualTo(String.format("build:Name@%s[17:43]", generateNewKytheUriBuilder(tickets).setPath("/corpus_root/foo/BUILD").build().toString()));
    assertThat(tickets.external("java_library")).isEqualTo("build:java_library");
    assertThat(tickets.getPackageName()).isEqualTo("EXT/foo");
    assertThat(tickets.getBasePackageName()).isEqualTo("foo");
  }

  @Test
  public void generatedBuildFile() {
    Tickets tickets = new Tickets(fs, "/corpus_root/", "foo/BUILD",
        "GENERATED" /* locationUriPrefix */, "" /* packageNamePrefix */, "kythe" /* corpus */);
    assertThat(tickets.localRule("Foo")).isEqualTo("build:foo:Foo");
    assertThat(tickets.localRuleDisplayName("Foo")).isEqualTo("foo:Foo");
    // assertThat(tickets.positional("Name", Location.fromFileAndOffsets(null, 17, 43)))
    //     .isEqualTo("build:Name@kythe://kythe/GENERATED/foo/BUILD[17:43]");
    assertThat(tickets.positional("Name", Location.fromFileAndOffsets(null, 17, 43)))
        .isEqualTo(String.format("build:Name@%s[17:43]", generateNewKytheUriBuilder(tickets).setPath("/corpus_root/GENERATED/foo/BUILD").build().toString()));
    assertThat(tickets.external("java_library")).isEqualTo("build:java_library");
    assertThat(tickets.getPackageName()).isEqualTo("foo");
    assertThat(tickets.getBasePackageName()).isEqualTo("foo");
  }

  /**
   * Generates a new KytheURI builder, auto-matically populated with information
   * from the specified {@link Tickets} tickets instance.
   *
   * @param tickets The tickets instance which default builder values will be
   * drawn from.
   * @return A new {@link Builder} builder instance.
   */
  private Builder generateNewKytheUriBuilder(Tickets tickets) {
    Builder result = KytheURI.newBuilder();
    result.setLanguage(Tickets.BUILD_FILE_LANGUAGE);
    if (tickets.getCorpusLabel() != null) {
      result.setCorpus(tickets.getCorpusLabel());
    }

    if (tickets.getCorpusRoot() != null) {
      result.setRoot(tickets.getCorpusRoot().getPathString());
    }

    return result;
  }
}
