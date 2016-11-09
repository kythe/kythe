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

import junit.framework.TestCase;
import junit.framework.TestSuite;

/** Tests {@link LabelUtil} */
public class LabelUtilTest extends TestCase {

  public void testCanonicalLabel_PackageAndRule() {
    assertEquals("foo/bar:baz",
        LabelUtil.canonicalize("//foo/bar:baz", "foo/bar"));
  }

  public void testCanonicalLabel_PackageOnly() {
    assertEquals("foo/bar:bar",
        LabelUtil.canonicalize("//foo/bar", "foo/bar"));
  }

  public void testCanonicalLabel_RuleOnly() {
    assertEquals("foo/bar:baz",
        LabelUtil.canonicalize(":baz", "foo/bar"));
  }

  public void testCanonicalLabel_NoColonRule() {
    assertEquals("foo/bar:baz",
        LabelUtil.canonicalize("baz", "foo/bar"));
  }

  public void testCanonicalLabel_WithADash() {
    assertEquals("foo/bar:baz-phred",
        LabelUtil.canonicalize(":baz-phred", "foo/bar"));
  }

  public void testCanonicalLabel_WithADashAndPeriod() {
    assertEquals("foo/bar-baz:bang.swz",
        LabelUtil.canonicalize(":bang.swz", "foo/bar-baz"));
  }

  public void testCanonicalLabel_WithADashAndPeriodFromOtherPackage() {
    assertEquals("abc/def-ghi:jkl.mno",
        LabelUtil.canonicalize("//abc/def-ghi:jkl.mno", "foo/bar"));
  }

  public void testCanonicalLabel_ExternalPackageAndRule() {
    assertEquals("abc/def:ghi",
        LabelUtil.canonicalize("//abc/def:ghi", "foo/bar"));
  }

  public void testCanonicalLabel_ExternalPackageOnly() {
    assertEquals("abc/def:def",
        LabelUtil.canonicalize("//abc/def", "foo/bar"));
  }

  public void testCanonicalLabel_LongerExternalPackageOnly() {
    assertEquals("abc/def/ghi/jkl/mno/pqr/stu/vwx:vwx",
        LabelUtil.canonicalize("//abc/def/ghi/jkl/mno/pqr/stu/vwx", "foo/bar"));
  }

  public static TestSuite suite() {
    return new TestSuite(LabelUtilTest.class);
  }
}
