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

package com.google.devtools.kythe.util;

import com.google.devtools.kythe.proto.Storage.VName;
import java.net.URISyntaxException;
import junit.framework.TestCase;

/** This class tests {@link KytheURI}. */
public class KytheURITest extends TestCase {

  public void testParse() throws URISyntaxException {
    // Empty URIs.
    assertSame(KytheURI.EMPTY, parse(""));
    assertSame(KytheURI.EMPTY, parse("kythe:"));
    assertSame(KytheURI.EMPTY, parse("kythe://"));

    // Individual components.
    assertEquals(builder().setSignature("sig").build(), parse("#sig"));
    assertEquals(builder().setSignature("sig").build(), parse("kythe:#sig"));
    assertEquals(builder().setCorpus("corpus").build(), parse("kythe://corpus"));
    assertEquals(builder().setCorpus("corpus").build(), parse("kythe://corpus/"));
    assertEquals(
        builder().setCorpus("corpus/with/path").build(), parse("kythe://corpus/with/path"));
    assertEquals(builder().setCorpus("corpus/with/path").build(), parse("//corpus/with/path"));
    assertEquals(builder().setRoot("R").build(), parse("kythe:?root=R"));
    assertEquals(builder().setPath("P").build(), parse("kythe:?path=P"));
    assertEquals(builder().setLanguage("L").build(), parse("kythe:?lang=L"));

    // Multiple attributes, with permutation of order.
    assertEquals(builder().setRoot("R").setLanguage("L").build(), parse("kythe:?lang=L?root=R"));
    assertEquals(
        builder().setRoot("R").setLanguage("L").setPath("P").build(),
        parse("kythe:?lang=L?path=P?root=R"));
    assertEquals(
        builder().setRoot("R").setLanguage("L").setPath("P").build(),
        parse("kythe:?root=R?path=P?lang=L"));

    // Everything.
    assertEquals(
        new KytheURI("sig", "bitbucket.org/creachadair/stringset", "blah", "stringset.go", "go"),
        parse(
            "kythe://bitbucket.org/creachadair/stringset?path=stringset.go?lang=go?root=blah#sig"));
    assertEquals(
        new KytheURI("", "libstdc++", "/usr/include/c++/4.8", "bits/basic_string.h", "c++"),
        parse(
            "kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/basic_string.h?root=/usr/include/c%2B%2B/4.8"));
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
    String hairyUri =
        "kythe://libstdc%2B%2B?lang=c%2B%2B?path=bits/basic_string.h?root=/usr/include/c%2B%2B/4.8";
    checkToString(hairyUri, hairyUri);

    // Path cleaning
    checkToString(
        "kythe://corpus/name/with/path",
        "kythe://corpus/name/with/path",
        "kythe://corpus/name///with//path",
        "kythe://corpus/name///with/./../with/path");
    checkToString("kythe://a/c#sig", "kythe://a/b/../c#sig", "kythe://a/./d/.././c#sig");
  }

  private void checkToString(String expected, String... cases) throws URISyntaxException {
    for (String str : cases) {
      assertEquals("KytheURI.parse(\"" + str + "\").toString()", expected, parse(str).toString());
    }
  }

  public void testGetters() throws URISyntaxException {
    String
        signature = "magic school truck",
        corpus = "com.crazyTown-1.20_PROTOTYPE",
        path = "usa/2.0",
        root = null,
        lang = "";
    KytheURI uri = new KytheURI(signature, corpus, root, path, lang);
    assertEquals(signature, uri.getSignature());
    assertEquals(corpus, uri.getCorpus());
    assertEquals("", uri.getRoot()); // nullToEmpty used
    assertEquals(path, uri.getPath());
    assertEquals(lang, uri.getLanguage());
  }

  public void testToVName() throws URISyntaxException {
    String
        signature = "magic school truck",
        corpus = "crazyTown",
        path = "usa/2.0",
        root = null,
        lang = "c++";
    VName vname = new KytheURI(signature, corpus, root, path, lang).toVName();
    assertEquals(signature, vname.getSignature());
    assertEquals(corpus, vname.getCorpus());
    assertEquals("", vname.getRoot()); // Proto fields are never null
    assertEquals(path, vname.getPath());
    assertEquals(lang, vname.getLanguage());
  }

  private static KytheURI.Builder builder() {
    return KytheURI.newBuilder();
  }

  private static KytheURI parse(String str) throws URISyntaxException {
    return KytheURI.parse(str);
  }
}
