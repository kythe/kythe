/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.extractors.java;

import static com.google.common.truth.Truth.assertThat;
import static java.nio.file.StandardCopyOption.REPLACE_EXISTING;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.extractors.shared.FileVNames;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileInfo;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.proto.Storage.VNameRewriteRule;
import com.google.devtools.kythe.proto.Storage.VNameRewriteRules;
import com.google.protobuf.Any;
import com.google.protobuf.InvalidProtocolBufferException;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import junit.framework.TestCase;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Tests for {@link JavaCompilationExtractor}. */
// TODO(schroederc): replace asserts with Truth#assertThat
public class JavaExtractorTest extends TestCase {

  private static final String TEST_DATA_DIR =
      "kythe/javatests/com/google/devtools/kythe/extractors/java/testdata";

  private static final String CORPUS = "testCorpus";
  private static final String TARGET1 = "target1";

  private static final ImmutableList<String> EMPTY = ImmutableList.of();

  private static final String JDK_ANNOTATION_PATH = "!CLASS_PATH_JAR!";
  private static final FileInfo GENERATED_ANNOTATION_CLASS =
      FileInfo.newBuilder()
          .setPath(join(JDK_ANNOTATION_PATH, "javax/annotation/Generated.class"))
          // This digest depends on the external org_apache_tomcat_tomcat_annotations_api
          // dependency.  If it is updated, this will also need to change.
          .setDigest("33d95d043232aa7dc76f00e831495f9b0e89f9c3f6fe440660390514624ec08c")
          .build();

  /** Tests the basic case of indexing a java compilation with two sources. */
  public void testJavaExtractorSimple() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = testFiles("/pkg/A.java", "/pkg/B.java");

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit.getVName().getCorpus()).isEqualTo(CORPUS);
    assertThat(unit.getWorkingDirectory()).isEqualTo("/root");
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);

    // With the expected sources as explicit sources.
    assertThat(unit.getSourceFileCount()).isEqualTo(2);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).containsExactly(TEST_DATA_DIR);
  }

  /** Tests the extraction with a {@link FileVNames}. */
  public void testJavaExtractorFileVNames() throws Exception {
    JavaCompilationUnitExtractor java =
        new JavaCompilationUnitExtractor(
            FileVNames.fromProto(
                VNameRewriteRules.newBuilder()
                    .addRule(
                        VNameRewriteRule.newBuilder()
                            .setPattern("(.*A.*)")
                            .setVName(VName.newBuilder().setCorpus("A").setPath("@1@").build()))
                    .addRule(
                        VNameRewriteRule.newBuilder()
                            .setPattern("(.*B.*)")
                            .setVName(VName.newBuilder().setCorpus("B").setPath("@1@").build())
                            .build())
                    .build()));

    List<String> sources = testFiles("/pkg/A.java", "/pkg/B.java");

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    // source files are from different corpora, so unit is assigned default corpus of "kythe"
    assertThat(unit.getVName().getCorpus()).isEqualTo("kythe");
    assertThat(unit.getWorkingDirectory()).isEqualTo("/root");
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);

    // With the expected sources as explicit sources.
    assertThat(unit.getSourceFileCount()).isEqualTo(2);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(sources));

    // With the expected VNames based on the FileVNames configuration.
    assertThat(getVNames(unit.getRequiredInputList()))
        .containsAtLeast(
            VName.newBuilder().setCorpus("A").setPath(join(TEST_DATA_DIR, "/pkg/A.java")).build(),
            VName.newBuilder().setCorpus("B").setPath(join(TEST_DATA_DIR, "/pkg/B.java")).build());

    // And the correct sourcepath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).containsExactly(TEST_DATA_DIR);
  }

  /** Tests that metadata is included when a file specifies it. */
  public void testJavaExtractorMetadata() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Lists.newArrayList(join(TEST_DATA_DIR, "/pkg/M.java"));
    List<String> dependencies =
        Lists.newArrayList(
            join(TEST_DATA_DIR, "/pkg/M.java"), join(TEST_DATA_DIR, "/pkg/M.java.meta"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);

    // With the expected sources as explicit sources.
    assertThat(unit.getSourceFileCount()).isEqualTo(1);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0));

    // With the expected dependencies as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(dependencies, GENERATED_ANNOTATION_CLASS));

    // And the correct sourcepath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).containsExactly(TEST_DATA_DIR);
    assertThat(details.getClasspathList()).contains(JDK_ANNOTATION_PATH);
  }

  /** Tests indexing within a symlink root. */
  public void testJavaExtractorSymlinkRoot() throws Exception {
    Path symlinkRoot = Paths.get(System.getenv("TEST_TMPDIR"), "symlinkRoot");
    symlinkRoot.toFile().delete();
    Files.createSymbolicLink(symlinkRoot, Paths.get(".").toAbsolutePath());
    symlinkRoot.toFile().deleteOnExit();

    JavaCompilationUnitExtractor java =
        new JavaCompilationUnitExtractor(CORPUS, symlinkRoot.toString());

    List<String> sources =
        Lists.newArrayList(join(TEST_DATA_DIR, "/pkg/A.java"), join(TEST_DATA_DIR, "/pkg/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);

    // With the expected sources as explicit sources.
    assertThat(unit.getSourceFileList()).containsExactlyElementsIn(sources).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).containsExactly(TEST_DATA_DIR);
  }

  /**
   * Tests the basic case of indexing a java compilation where the sources live in different folders
   * eventhough they're in the same package.
   */
  public void testJavaExtractorTwoSourceDirs() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    // Pick two sources in the same package that live in different directories.
    List<String> sources =
        Lists.newArrayList(
            join(TEST_DATA_DIR, "one/pkg/A.java"), join(TEST_DATA_DIR, "two/pkg/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);

    // With the expected sources as explicit sources.
    assertThat(unit.getSourceFileCount()).isEqualTo(2);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation for both sources.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList())
        .containsExactly(join(TEST_DATA_DIR, "one"), join(TEST_DATA_DIR, "two"));
  }

  /**
   * Tests java compilation with classfiles in the classpath. Test ensures that only used classfiles
   * are stored.
   */
  public void testJavaExtractorClassFiles() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    // Index the specified sources
    List<String> sources = Lists.newArrayList(join(TEST_DATA_DIR, "child/derived/B.java"));

    String parentCp = join(TEST_DATA_DIR, "parent") + "/";
    List<String> classpath = Lists.newArrayList(parentCp);
    String classFile = join(parentCp, "base/A.class");

    // Index the specified sources and classes from the classpath
    CompilationDescription description =
        java.extract(TARGET1, sources, classpath, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(1);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0)).inOrder();

    // Ensure the right class files are picked up from the classpath.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(Arrays.asList(sources.get(0), classFile)));

    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).containsExactly(join(TEST_DATA_DIR, "child"));
    // Ensure the classpath is set for replay.
    assertThat(details.getClasspathList()).containsExactly(parentCp);
  }

  /**
   * Tests java compilation with classfiles in a jarfile on the classpath. Ensures that only used
   * classfiles are stored.
   */
  public void testJavaExtractorJar() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    // Index the specified sources.
    List<String> sources = Lists.newArrayList(join(TEST_DATA_DIR, "child/derived/B.java"));

    String parentCp = join(TEST_DATA_DIR, "jarred.jar");
    List<String> classpath = Lists.newArrayList(parentCp);

    // Index the specified sources and classes from inside jar on the classpath.
    CompilationDescription description =
        java.extract(TARGET1, sources, classpath, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileList()).containsExactlyElementsIn(sources).inOrder();

    // The classpath is adjusted to start with !CLASS_PATH_JAR!
    String classFile = join("!CLASS_PATH_JAR!", "base/A.class");

    // Ensure the right class files are picked up from the classpath from within the jar.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeast(
            makeFileInfo(sources.get(0)),
            makeFileInfo(classFile, join(TEST_DATA_DIR, "parent/base/A.class")));

    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).hasSize(1);
    assertThat(details.getSourcepathList().get(0)).isEqualTo(join(TEST_DATA_DIR, "child"));
    // Ensure the magic !CLASS_PATH_JAR! classpath is added.
    assertThat(details.getClasspathList()).containsExactly("!CLASS_PATH_JAR!");
  }

  /**
   * Tests case where compile errors exist in the sources under compilation. Compilation should
   * still be created.
   */
  public void testJavaExtractorCompileError() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Lists.newArrayList(join(TEST_DATA_DIR, "/error/Crash.java"));

    // Index the specified sources, reporting compilation failure.
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(getInfos(unit.getRequiredInputList())).contains(makeFileInfo(sources.get(0)));
    assertThat(unit.getSourceFileCount()).isEqualTo(1);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0)).inOrder();

    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).hasSize(1);
    assertThat(details.getSourcepathList().get(0)).isEqualTo(TEST_DATA_DIR);
    assertThat(details.getClasspathList()).isEmpty();
  }

  /** Tests that dependent files are ordered correctly. */
  public void testJavaExtractorOrderedDependencies() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources =
        Lists.newArrayList(
            join(TEST_DATA_DIR, "/deps/A.java"),
            join(TEST_DATA_DIR, "/deps/C.java"),
            join(TEST_DATA_DIR, "/deps/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(3);
    assertThat(unit.getSourceFileList())
        .containsExactly(sources.get(0), sources.get(1), sources.get(2))
        .inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeast(
            makeFileInfo(sources.get(0)),
            makeFileInfo(sources.get(2)),
            makeFileInfo(sources.get(1)))
        .inOrder();
  }

  /**
   * Tests the case where one of the sources doesn't follow the java pattern of naming the path to
   * the source file after the package name.
   */
  public void testJavaExtractorNonConformingPath() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources =
        Lists.newArrayList(
            join(TEST_DATA_DIR, "/path/sub/A.java"), join(TEST_DATA_DIR, "/path/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(2);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).hasSize(1);
    assertThat(details.getSourcepathList().get(0)).isEqualTo(TEST_DATA_DIR);
  }

  /**
   * Tests the case where one of the sources defines a package only type that is not conforming to
   * the classname matching the filename.
   */
  public void testJavaExtractorAdditionalTypeDefinitions() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources =
        Lists.newArrayList(
            join(TEST_DATA_DIR, "/hitchhikers/A.java"), join(TEST_DATA_DIR, "/hitchhikers/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(2);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeastElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).containsExactly(TEST_DATA_DIR);
  }

  /**
   * Tests that unsupported flags do not crash the extractor and source/header destination options
   * are not present in the resulting {@link CompilationUnit}.
   */
  public void testJavaExtractorArguments() throws Exception {
    String testDir = System.getenv("TEST_TMPDIR");
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS, testDir);

    File processorFile =
        Paths.get(
                "kythe/javatests/com/google/devtools/kythe/extractors/java/SillyProcessor_deploy.jar")
            .toFile();
    if (!processorFile.exists()) {
      throw new AssertionError("SillyProcessor_deploy.jar does not exist");
    }

    List<String> origSources = testFiles("processor/Silly.java", "processor/SillyUser.java");

    List<Path> outputDirs =
        Arrays.asList(
            Paths.get(testDir, "output-gensrc.jar.files"),
            Paths.get(testDir, "output-genhdr.jar.files"),
            Paths.get(testDir, "classes"));

    List<String> processorpath = Arrays.asList(processorFile.getPath());
    List<String> processors = Arrays.asList("processor.SillyProcessor");
    List<String> options =
        Arrays.asList(
            "-Xdoclint:-Xdoclint:all/private", // ensure this unsupported flag is saved
            "-s",
            outputDirs.get(0).toString(),
            "-h",
            outputDirs.get(1).toString(),
            "-g:lines", // ensure this conjoined arg is handled correctly
            "-d",
            outputDirs.get(2).toString());

    for (Path dir : outputDirs) {
      dir.toFile().mkdir();
    }

    // Copy sources from runfiles into test dir
    List<String> testSources = new ArrayList<>();
    for (String source : origSources) {
      Path destFile = Paths.get(testDir).resolve(source);
      Files.createDirectories(destFile.getParent());
      Files.copy(Paths.get(source), destFile, REPLACE_EXISTING);
      testSources.add(destFile.toString());
    }

    // Index the specified sources
    CompilationDescription description =
        java.extract(
            TARGET1,
            testSources,
            EMPTY,
            EMPTY,
            EMPTY,
            processorpath,
            processors,
            options,
            "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();

    // Check that the -s, and -h flags have been removed from the compilation's arguments, but -d
    // preserved (it is required by modular builds).
    assertThat(unit.getArgumentList())
        .containsExactly("-Xdoclint:-Xdoclint:all/private", "-g:lines")
        .inOrder();
  }

  /** Tests that targets that contain annotation processors are indexed correctly. */
  public void testJavaExtractorAnnotationProcessing() throws Exception {
    String testDir = System.getenv("TEST_TMPDIR");
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS, testDir);

    File processorFile =
        Paths.get(
                "kythe/javatests/com/google/devtools/kythe/extractors/java/SillyProcessor_deploy.jar")
            .toFile();
    if (!processorFile.exists()) {
      throw new AssertionError("SillyProcessor_deploy.jar does not exist");
    }

    List<String> origSources = testFiles("processor/Silly.java", "processor/SillyUser.java");

    List<String> processorpath = Arrays.asList(processorFile.getPath());
    List<String> processors = Arrays.asList("processor.SillyProcessor");
    Path genSrcDir = Paths.get(testDir, "output-gensrc.jar.files");
    List<String> options = Arrays.asList("-s", genSrcDir.toString());
    genSrcDir.toFile().mkdir();

    // Copy sources from runfiles into test dir
    List<String> testSources = new ArrayList<>();
    for (String source : origSources) {
      Path destFile = Paths.get(testDir).resolve(source);
      Files.createDirectories(destFile.getParent());
      Files.copy(Paths.get(source), destFile, REPLACE_EXISTING);
      testSources.add(destFile.toString());
    }

    // Index the specified sources
    CompilationDescription description =
        java.extract(
            TARGET1,
            testSources,
            EMPTY,
            EMPTY,
            EMPTY,
            processorpath,
            processors,
            options,
            "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);

    String sillyGenerated = "output-gensrc.jar.files/processor/SillyGenerated.java";
    assertThat(unit.getSourceFileList())
        .containsExactly(origSources.get(0), origSources.get(1), sillyGenerated)
        .inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeast(
            makeFileInfo(origSources.get(0)),
            makeFileInfo(origSources.get(1)),
            makeFileInfo(sillyGenerated, Paths.get(testDir, sillyGenerated).toString()));

    // And the correct sourcepath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList())
        .containsExactly(TEST_DATA_DIR, "output-gensrc.jar.files");
  }

  /** Tests that the extractor doesn't fall over when it's provided with no sources. */
  public void testNoSources() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Collections.emptyList();
    List<String> bootclasspath = testFiles("empty/fake-rt.jar");

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, bootclasspath, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();

    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(0);
    assertThat(unit.getRequiredInputCount()).isEqualTo(0);

    // And the correct classpath set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).isEmpty();
    assertThat(details.getClasspathList()).isEmpty();
  }

  /**
   * Tests the case where one of the sources defines a package only type that is not conforming to
   * the classname matching the filename.
   */
  public void testEmptyCompilationUnit() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = testFiles("empty/Empty.java");
    List<String> bootclasspath = testFiles("empty/fake-rt.jar");

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, bootclasspath, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(1);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0)).inOrder();

    List<String> requiredPaths = new ArrayList<>();
    for (FileInput input : unit.getRequiredInputList()) {
      requiredPaths.add(input.getInfo().getPath());
    }
    assertThat(requiredPaths)
        .containsAtLeast(sources.get(0), "!PLATFORM_CLASS_PATH_JAR!/java/lang/Fake.class");

    assertThat(unit.getArgumentList())
        .containsNoneOf("-bootclasspath", "-sourcepath", "-cp", "-classpath");

    // And the correct source/class locations set to replay the compilation.
    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).isEmpty();
    assertThat(details.getBootclasspathList()).containsExactly("!PLATFORM_CLASS_PATH_JAR!");
  }

  public void testSourceInJar() throws ExtractionException, InvalidProtocolBufferException {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = testFiles("src_in_jar/C1.java");
    ImmutableList<String> classpath = ImmutableList.of(join(TEST_DATA_DIR, "src_in_jar/pack.jar"));

    CompilationDescription description =
        java.extract(TARGET1, sources, classpath, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(1);
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0)).inOrder();

    // Ensure the source jar and the class jar are included in the required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsAtLeast(
            makeFileInfo(sources.get(0)),
            makeFileInfo(
                "!CLASS_PATH_JAR!/pack2/C3.class",
                join(TEST_DATA_DIR, "src_in_jar/pack2/C3.class")),
            makeFileInfo(
                "!SOURCE_JAR!/pack/C2.java", join(TEST_DATA_DIR, "src_in_jar/pack/C2.java")));

    JavaDetails details = getJavaDetails(unit);
    assertThat(details.getSourcepathList()).containsExactly("!SOURCE_JAR!");
    // Ensure the classpath is set for replay.
    assertThat(details.getClasspathList()).containsExactly("!CLASS_PATH_JAR!");
  }

  public void testSystemDir() throws ExtractionException, InvalidProtocolBufferException {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = testFiles("/pkg/A.java", "/pkg/B.java");

    java.useSystemDirectory(join(TEST_DATA_DIR, "/system_modules"));

    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileCount()).isEqualTo(2);
    assertThat(unit.getSourceFileList()).containsExactlyElementsIn(sources).inOrder();

    List<String> requiredInputs =
        unit.getRequiredInputList().stream()
            .map(f -> f.getInfo().getPath())
            .collect(Collectors.toList());
    assertThat(requiredInputs)
        .containsAtLeast(
            join(TEST_DATA_DIR, "/pkg/A.java"),
            join(TEST_DATA_DIR, "/pkg/B.java"),
            join(TEST_DATA_DIR, "/system_modules/lib/jrt-fs.jar"),
            join(TEST_DATA_DIR, "/system_modules/lib/modules"));
  }

  private List<String> testFiles(String... files) {
    List<String> res = new ArrayList<>();
    for (String file : files) {
      res.add(join(TEST_DATA_DIR, file));
    }
    return res;
  }

  private static List<FileInfo> getInfos(List<FileInput> files) {
    return Lists.transform(files, FileInput::getInfo);
  }

  private static List<VName> getVNames(List<FileInput> files) {
    return Lists.transform(files, FileInput::getVName);
  }

  private static List<FileInfo> getExpectedInfos(List<String> files, FileInfo... extra) {
    return Stream.concat(files.stream().map(JavaExtractorTest::makeFileInfo), Stream.of(extra))
        .collect(Collectors.toList());
  }

  private static FileInfo makeFileInfo(String path) {
    try {
      return FileInfo.newBuilder()
          .setPath(path)
          .setDigest(ExtractorUtils.digestForPath(path))
          .build();
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  private static FileInfo makeFileInfo(String path, String localPath) {
    try {
      return FileInfo.newBuilder()
          .setPath(path)
          .setDigest(ExtractorUtils.digestForPath(localPath))
          .build();
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  private @Nullable JavaDetails getJavaDetails(CompilationUnit unit)
      throws InvalidProtocolBufferException {
    for (Any any : unit.getDetailsList()) {
      if (any.getTypeUrl().equals(JavaCompilationUnitExtractor.JAVA_DETAILS_URL)) {
        return JavaDetails.parseFrom(any.getValue());
      }
    }
    return null;
  }

  private static String join(String base, String... paths) {
    return Paths.get(base, paths).toString();
  }
}
