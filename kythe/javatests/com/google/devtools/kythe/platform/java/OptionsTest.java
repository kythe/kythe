/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.platform.java;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.collect.Lists;
import com.sun.tools.javac.api.JavacTool;
import java.util.List;
import junit.framework.TestCase;

/** Unit tests for {@link JavacOptionsUtils}. */
public class OptionsTest extends TestCase {

  public void testRemoveUnsupportedOptions_depAnn() {
    final String lintOption = "-Xlint:-dep-ann";

    List<String> rawArgs = Lists.newArrayList(lintOption, "--class-path", "some/class/path");
    assertThat(JavacOptionsUtils.removeUnsupportedOptions(rawArgs))
        .containsExactly("--class-path", "some/class/path")
        .inOrder();
  }
}
