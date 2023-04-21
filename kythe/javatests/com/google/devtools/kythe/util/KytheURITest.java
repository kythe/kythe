/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.util;

import static com.google.common.truth.Truth.assertThat;
import static com.google.common.truth.Truth.assertWithMessage;

import com.google.devtools.kythe.proto.CorpusPath;
import com.google.devtools.kythe.proto.Storage.VName;
import java.net.URISyntaxException;
import junit.framework.TestCase;

/** This class tests {@link KytheURI}. */
public class KytheURITest extends TestCase {

  public void testParse() throws URISyntaxException {
    // Empty URIs.
    assertThat(parse("")).isSameInstanceAs(KytheURI.EMPTY);
    assertThat(parse("kythe:")).isSameInstanceAs(KytheURI.EMPTY);
    assertThat(parse("kythe://")).isSameInstanceAs(KytheURI.EMPTY);

    // Individual components.
    assertThat(parse("#sig")).isEqualTo(builder().setSignature("sig").build());
    assertThat(parse("kythe:#sig")).isEqualTo(builder().setSignature("sig").build());
    assertThat(parse("kythe://corpus")).isEqualTo(builder().setCorpus("corpus").build());
    assertThat(parse("kythe://corpus/")).isEqualTo(builder().setCorpus("corpus/").build());
    assertThat(parse("kythe://corpus/with/path"))
        .isEqualTo(builder().setCorpus("corpus/with/path").build());
    assertThat(parse("//corpus/with/path"))
        .isEqualTo(builder().setCorpus("corpus/with/path").build());
    assertThat(parse("kythe:?root=R")).isEqualTo(builder().setRoot("R").build());
    assertThat(parse("kythe:?path=P")).isEqualTo(builder().setPath("P").build());
    assertThat(parse("kythe:?lang=L")).isEqualTo(builder().setLanguage("L").build());

    // Multiple attributes, with permutation of order.
    assertThat(parse("kythe:?lang=L?root=R"))
        .isEqualTo(builder().setRoot("R").setLanguage("L").build());
    assertThat(parse("kythe:?lang=L?path=P?root=R"))
        .isEqualTo(builder().setRoot("R").setLanguage("L").setPath("P").build());
    assertThat(parse("kythe:?root=R?path=P?lang=L"))
        .isEqualTo(builder().setRoot("R").setLanguage("L").setPath("P").build());

    // Everything.
    assertThat(
            parse(
                "kythe://bitbucket.org/creachadair/stringset?path=stringset.go?lang=go?root=blah#sig"))
        .isEqualTo(
            new KytheURI(
                "sig", "bitbucket.org/creachadair/stringset", "blah", "stringset.go", "go"));
    assertThat(
            parse(
                "kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/basic_string.h?root=/usr/include/c%2B%2B/4.8"))
        .isEqualTo(
            new KytheURI("", "libstdc++", "/usr/include/c++/4.8", "bits/basic_string.h", "c++"));
  }

  public void testToString() throws URISyntaxException {
    // Empty URIs
    checkToString("kythe:", "", "kythe:", "kythe://", "kythe://#");

    // Order of attributes is normalized (lang, path, root)
    checkToString(
        "kythe:?lang=L?path=P?root=R",
        "kythe:?root=R?path=P?lang=L",
        "kythe:?root=R?lang=L?path=P",
        "kythe:?lang=L?path=P?root=R",
        "kythe://?lang=L?path=P?root=R#");

    // Test various characters in the hostname
    checkToString(
        "kythe://com.crazyTown-1.20_PROTOTYPE/blah?path=P",
        "kythe://com.crazyTown-1.20_PROTOTYPE/blah?path=P");

    // Check escaping
    checkToString("kythe:?path=P", "kythe://?path=%50");
    checkToString("kythe:?path=%20", "kythe://?path=%20");
    checkToString("kythe:?path=a%2Bb", "kythe://?path=a+b");
    checkToString("kythe:?path=%2B", "kythe://?path=%2B");
    checkToString("kythe://somecorpus//branch", "kythe://somecorpus//branch");
    String hairyUri =
        "kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/basic_string.h?root=/usr/include/c%2B%2B/4.8";
    checkToString(hairyUri, hairyUri);

    // Corpus labels are not normalized, even if they "look like" paths.
    checkToString("kythe://corpus/name/with/path", "kythe://corpus/name/with/path");
    checkToString("kythe://corpus/name///with//path", "kythe://corpus/name///with//path");
    checkToString(
        "kythe://corpus/name///with/./../with/path", "kythe://corpus/name///with/./../with/path");
    checkToString("kythe://a/c#sig", "kythe://a/c#sig");
    checkToString("kythe://a/b/../c#sig", "kythe://a/b/../c#sig");
    checkToString("kythe://a/./d/.././c#sig", "kythe://a/./d/.././c#sig");

    // Paths are cleaned.
    checkToString(
        "kythe://a?path=c#sig", "kythe://a?path=b/../c#sig", "kythe://a?path=./d/.././c#sig");
  }

  public void testCorpusPath() {
    assertThat(KytheURI.asString(cp("", "", ""))).isEqualTo("kythe:");
    assertThat(KytheURI.asString(cp("c", "", ""))).isEqualTo("kythe://c");
    assertThat(KytheURI.asString(cp("c", "r", ""))).isEqualTo("kythe://c?root=r");
    assertThat(KytheURI.asString(cp("c", "r", "p"))).isEqualTo("kythe://c?path=p?root=r");
    assertThat(KytheURI.asString(cp("c", "", "p"))).isEqualTo("kythe://c?path=p");
    assertThat(KytheURI.asString(cp("", "r", "p"))).isEqualTo("kythe:?path=p?root=r");
    assertThat(KytheURI.asString(cp("", "", "p"))).isEqualTo("kythe:?path=p");
  }

  public void testToStringGoCompatibility() {
    // Test cases added when an incompatibility with Go's kytheuri library is found.
    assertThat(builder().setSignature("a=").build().toString()).isEqualTo("kythe:#a%3D");
    assertThat(builder().setCorpus("kythe").setSignature("a=").build().toString())
        .isEqualTo("kythe://kythe#a%3D");
    assertThat(builder().setSignature("広").build().toString()).isEqualTo("kythe:#%E5%BA%83");

    assertThat(builder().setCorpus("kythe").setPath("unrooted/path").build().toString())
        .isEqualTo("kythe://kythe?path=unrooted/path");
    assertThat(builder().setCorpus("kythe").setPath("/rooted/path").build().toString())
        .isEqualTo("kythe://kythe?path=/rooted/path");
    assertThat(builder().setCorpus("kythe").setPath("//rooted//path").build().toString())
        .isEqualTo("kythe://kythe?path=/rooted/path");
  }

  private void checkToString(String expected, String... cases) {
    for (String str : cases) {
      assertWithMessage("KytheURI.parse(\"" + str + "\").toString()")
          .that(parse(str).toString())
          .isEqualTo(expected);
    }
  }

  public void testGetters() throws URISyntaxException {
    String signature = "magic school truck";
    String corpus = "crazyTown";
    String path = "usa/2.0";
    String root = null;
    String lang = "c++";
    KytheURI uri = new KytheURI(signature, corpus, root, path, lang);
    assertThat(uri.getSignature()).isEqualTo(signature);
    assertThat(uri.getCorpus()).isEqualTo(corpus);
    assertThat(uri.getRoot()).isEmpty(); // nullToEmpty used
    assertThat(uri.getPath()).isEqualTo(path);
    assertThat(uri.getLanguage()).isEqualTo(lang);
  }

  public void testToVName() throws URISyntaxException {
    String signature = "magic school truck";
    String corpus = "crazyTown";
    String path = "usa/2.0";
    String root = null;
    String lang = "c++";
    VName vname = new KytheURI(signature, corpus, root, path, lang).toVName();
    assertThat(vname.getSignature()).isEqualTo(signature);
    assertThat(vname.getCorpus()).isEqualTo(corpus);
    assertThat(vname.getRoot()).isEmpty(); // Proto fields are never null
    assertThat(vname.getPath()).isEqualTo(path);
    assertThat(vname.getLanguage()).isEqualTo(lang);
  }

  public void testToCorpusPath() throws URISyntaxException {
    String signature = "magic school truck";
    String corpus = "crazyTown";
    String path = "usa/2.0";
    String root = null;
    String lang = "c++";
    CorpusPath cp = new KytheURI(signature, corpus, root, path, lang).toCorpusPath();
    assertThat(cp.getCorpus()).isEqualTo(corpus);
    assertThat(cp.getRoot()).isEmpty(); // Proto fields are never null
    assertThat(cp.getPath()).isEqualTo(path);
  }

  public void testParseErrors() {
    String[] tests =
        new String[] {
          "badscheme:",
          "badscheme://corpus",
          "badscheme:?path=path",
          "kythe:#sig1#sig2",
          "kythe:?badparam=val"
        };
    for (String test : tests) {
      try {
        KytheURI.parse(test);
        fail();
      } catch (RuntimeException e) {
        // pass test
      }
    }
  }

  private static KytheURI.Builder builder() {
    return KytheURI.newBuilder();
  }

  private static KytheURI parse(String str) {
    return KytheURI.parse(str);
  }

  private static CorpusPath cp(String corpus, String root, String path) {
    return CorpusPath.newBuilder().setCorpus(corpus).setRoot(root).setPath(path).build();
  }
}
