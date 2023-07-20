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

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;
import com.google.devtools.kythe.platform.java.JavacOptionsUtils.ModifiableOptions;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.protobuf.Any;
import com.sun.tools.javac.code.Source;
import com.sun.tools.javac.main.Option;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class OptionsTest {
  @Test
  public void removeUnsupportedOptions_depAnn() {
    final String lintOption = "-Xlint:-dep-ann";

    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of(lintOption, "--class-path", "some/class/path"));
    assertThat(args.removeUnsupportedOptions().build())
        .containsExactly("--class-path", "some/class/path")
        .inOrder();
  }

  @Test
  public void removeUnsupportedOptions_missingArgFine() {
    ModifiableOptions args = ModifiableOptions.of(ImmutableList.of("--class-path"));
    assertThat(args.removeUnsupportedOptions().build()).containsExactly("--class-path").inOrder();
  }

  @Test
  public void removeOptions_singles() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-prompt", "-doe", "-moreinfo", "--class-path"));
    assertThat(args.removeOptions(ImmutableSet.of(Option.DOE)).build())
        .containsExactly("-prompt", "-moreinfo", "--class-path")
        .inOrder();
  }

  @Test
  public void removeOptions_positionalClobbers() {
    ModifiableOptions args =
        ModifiableOptions.of(
            ImmutableList.of("-prompt", "-doe", "--class-path", "not/a/real/path", "-moreinfo"));
    assertThat(args.removeOptions(ImmutableSet.of(Option.CLASS_PATH)).build())
        .containsExactly("-prompt", "-doe", "-moreinfo")
        .inOrder();
  }

  @Test
  public void removeOptions_adjacentDoesntClobber() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-prompt", "-doe", "-proc:none", "-moreinfo"));
    assertThat(args.removeOptions(ImmutableSet.of(Option.PROC)).build())
        .containsExactly("-prompt", "-doe", "-moreinfo")
        .inOrder();
  }

  @Test
  public void removeOptions_handlesAdjacentEquals() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-prompt", "-Djava.ext.dirs=not/a/dir", "-moreinfo"));
    assertThat(args.removeOptions(ImmutableSet.of(Option.DJAVA_EXT_DIRS)).build())
        .containsExactly("-prompt", "-moreinfo")
        .inOrder();
  }

  @Test
  public void keepOptions_singles() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-prompt", "-doe", "-moreinfo", "--class-path"));
    assertThat(args.keepOptions(ImmutableSet.of(Option.DOE, Option.PROMPT)).build())
        .containsExactly("-prompt", "-doe")
        .inOrder();
  }

  @Test
  public void keepOptions_positionalClobbers() {
    ModifiableOptions args =
        ModifiableOptions.of(
            ImmutableList.of(
                "-proc:none",
                "--processor-path",
                "fake/path",
                "--class-path",
                "not/a/real/path",
                "-moreinfo"));
    assertThat(args.keepOptions(ImmutableSet.of(Option.CLASS_PATH)).build())
        .containsExactly("--class-path", "not/a/real/path")
        .inOrder();
  }

  // Note that previous behavior of this function *did* clobber unrelated next args when an
  // adjacent ':' arg was specified.
  @Test
  public void keepOptions_adjacentDoesntClobber() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-prompt", "-doe", "-proc:none", "-moreinfo"));
    assertThat(args.keepOptions(ImmutableSet.of(Option.PROC)).build())
        .containsExactly("-proc:none")
        .inOrder();
  }

  @Test
  public void replaceOptionValue_replacesExtantValue() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-prompt", "-d", "/some/random/path", "-moreinfo"));
    assertThat(args.replaceOptionValue(Option.D, "/another/random/path").build())
        .containsExactly("-prompt", "-d", "/another/random/path", "-moreinfo")
        .inOrder();
  }

  @Test
  public void updateWithJavaOptions_updatesBootClassPath() {
    ModifiableOptions args =
        ModifiableOptions.of(
            ImmutableList.of("--boot-class-path", "not/a/real/path:also/fake", "-doe"));
    ImmutableList<String> updatedArgs =
        args.updateWithJavaOptions(CompilationUnit.getDefaultInstance()).build();
    // Note the re-ordering.  Probably it doesn't matter, but enforce in test just to be sure.
    assertThat(updatedArgs).containsAtLeast("-doe", "--boot-class-path").inOrder();
    // Also verify that we kept paths passed in.
    int idx = updatedArgs.indexOf("--boot-class-path");
    assertThat(updatedArgs).hasSize(idx + 2);
    assertThat(updatedArgs.get(idx + 1)).startsWith("not/a/real/path:also/fake");
    assertThat(updatedArgs.get(idx + 1)).matches(".*(\\.jar|lib/modules)");
  }

  @Test
  public void updateWithJavaOptions_emptyDetailsAddsBootClassPath() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("--class-path", "not/a/real/path:also/fake", "-doe"));
    ImmutableList<String> updatedArgs =
        args.updateWithJavaOptions(CompilationUnit.getDefaultInstance()).build();
    assertThat(updatedArgs).containsAtLeast("--class-path", "-doe", "--boot-class-path").inOrder();
    assertThat(updatedArgs.get(updatedArgs.indexOf("--class-path") + 1))
        .startsWith("not/a/real/path:also/fake");

    assertThat(updatedArgs.get(updatedArgs.indexOf("--boot-class-path") + 1))
        .matches(".*(\\.jar|lib/modules)");
  }

  @Test
  public void updateWithJavaOptions_updatesPathsFromDetails() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("--class-path", "not/a/real/path:also/fake", "-doe"));

    CompilationUnit cu =
        CompilationUnit.newBuilder()
            .addDetails(
                Any.pack(
                    JavaDetails.newBuilder()
                        .addClasspath("class/from/details")
                        .addSourcepath("source/from/details")
                        .addBootclasspath("boot/from/details")
                        .build()))
            .build();
    ImmutableList<String> updatedArgs = args.updateWithJavaOptions(cu).build();
    // Note that the unspecified --source-path and --boot-class-path get added in from details.
    assertThat(updatedArgs)
        .containsAtLeast("-doe", "--class-path", "--source-path", "--boot-class-path")
        .inOrder();
    int classIdx = updatedArgs.indexOf("--class-path");
    assertThat(updatedArgs.get(classIdx + 1)).startsWith("class/from/details");
    // Should clobber the argument, leaving only the non-argument.
    assertThat(updatedArgs.get(classIdx + 1)).doesNotContain("not/a/real/path:also/fake");

    assertThat(updatedArgs.get(updatedArgs.indexOf("--source-path") + 1))
        .contains("source/from/details");
    assertThat(updatedArgs.get(updatedArgs.indexOf("--boot-class-path") + 1))
        .contains("boot/from/details");
  }

  @Test
  public void updateWithJavaOptions_updatesSomePathsFromDetails() {
    ModifiableOptions args = ModifiableOptions.of(ImmutableList.of("-doe"));

    CompilationUnit cu =
        CompilationUnit.newBuilder()
            .addDetails(
                Any.pack(JavaDetails.newBuilder().addSourcepath("source/from/details").build()))
            .build();
    ImmutableList<String> updatedArgs = args.updateWithJavaOptions(cu).build();
    // --source-path comes from details, --boot-class-path comes from system.
    assertThat(updatedArgs).containsAtLeast("-doe", "--source-path", "--boot-class-path").inOrder();
    assertThat(updatedArgs.get(updatedArgs.indexOf("--source-path") + 1))
        .matches("source/from/details");
    assertThat(updatedArgs.get(updatedArgs.indexOf("--boot-class-path") + 1))
        .matches(".*(\\.jar|lib/modules)");
  }

  @Test
  public void updateToMinimumSupportedSourceVersion_updatesSource() {
    // We can't test both --source and --source formats of the flag because it is only supported by
    // recent versions of java.
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-foo", "-source", Source.JDK1_2.name));

    assertThat(args.updateToMinimumSupportedSourceVersion().build())
        .containsExactly("-foo", Option.SOURCE.getPrimaryName(), Source.MIN.name)
        .inOrder();
  }

  @Test
  public void updateToMinimumSupportedSourceVersion_updatesSource1DotFormat() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-foo", "-source", Source.JDK1_2.name));

    assertThat(args.updateToMinimumSupportedSourceVersion().build())
        .containsExactly("-foo", Option.SOURCE.getPrimaryName(), Source.MIN.name)
        .inOrder();
  }

  @Test
  public void updateToMinimumSupportedSourceVersion_removeTarget() {
    ModifiableOptions args =
        ModifiableOptions.of(
            ImmutableList.of("-foo", "-source", Source.JDK1_2.name, "-target", Source.JDK1_2.name));

    assertThat(args.updateToMinimumSupportedSourceVersion().build())
        .containsExactly("-foo", Option.SOURCE.getPrimaryName(), Source.MIN.name)
        .inOrder();
  }

  @Test
  public void updateToMinimumSupportedSourceVersion_updatesMultipleSource() {
    ModifiableOptions args =
        ModifiableOptions.of(
            ImmutableList.of("-foo", "-source", Source.JDK1_2.name, "-source", Source.JDK1_3.name));

    assertThat(args.updateToMinimumSupportedSourceVersion().build())
        .containsExactly("-foo", Option.SOURCE.getPrimaryName(), Source.MIN.name)
        .inOrder();
  }

  @Test
  public void updateToMinimumSupportedSourceVersion_doesNotUpdateSupportedVersion() {
    ModifiableOptions args =
        ModifiableOptions.of(ImmutableList.of("-foo", "-source", Source.DEFAULT.name));

    assertThat(args.updateToMinimumSupportedSourceVersion().build())
        .containsExactly("-foo", Option.SOURCE.getPrimaryName(), Source.DEFAULT.name)
        .inOrder();
  }
}
