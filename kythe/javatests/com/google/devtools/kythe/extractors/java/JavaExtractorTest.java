/*
 * Copyright 2015 Google Inc. All rights reserved.
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

import com.google.common.base.Function;
import com.google.common.base.Throwables;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import com.google.devtools.kythe.common.PathUtil;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractorUtils;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit.FileInput;
import com.google.devtools.kythe.proto.Analysis.FileInfo;

import junit.framework.TestCase;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/** Tests for {@link JavaCompilationExtractor}. */
// TODO(schroederc): replace asserts with Truth#assertThat
public class JavaExtractorTest extends TestCase {

  private static final String TEST_DATA_DIR =
      "kythe/javatests/com/google/devtools/kythe/extractors/java/testdata";

  private static final String CORPUS = "testCorpus";
  private static final String TARGET1 = "target1";

  private static final List<String> EMPTY = ImmutableList.of();

  /**
   * Tests the basic case of indexing a java compilation with two sources.
   */
  public void testJavaExtractorSimple() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Lists.newArrayList(PathUtil.join(TEST_DATA_DIR,  "/pkg/A.java"),
        PathUtil.join(TEST_DATA_DIR, "/pkg/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());

    // With the expected sources as explicit sources.
    assertEquals(2, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactlyElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaArguments args = JavaArguments.parseArguments(unit);
    assertThat(args.getSourcepath()).containsExactly(TEST_DATA_DIR);
    assertThat(args.getClasspath()).isEmpty();
  }

  /** Tests indexing within a symlink root. */
  public void testJavaExtractorSymlinkRoot() throws Exception {
    Path symlinkRoot = Paths.get(System.getenv("TEST_TMPDIR"), "symlinkRoot");
    symlinkRoot.toFile().delete();
    Files.createSymbolicLink(symlinkRoot, Paths.get(".").toAbsolutePath());
    symlinkRoot.toFile().deleteOnExit();

    JavaCompilationUnitExtractor java =
        new JavaCompilationUnitExtractor(CORPUS, symlinkRoot.toString());

    List<String> sources = Lists.newArrayList(
        PathUtil.join(TEST_DATA_DIR, "/pkg/A.java"),
        PathUtil.join(TEST_DATA_DIR, "/pkg/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);

    // With the expected sources as explicit sources.
    assertThat(unit.getSourceFileList())
        .containsExactlyElementsIn(sources)
        .inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactlyElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaArguments args = JavaArguments.parseArguments(unit);
    assertThat(args.getSourcepath()).containsExactly(TEST_DATA_DIR);
    assertThat(args.getClasspath()).isEmpty();
  }

  /**
   * Tests the basic case of indexing a java compilation where the sources
   * live in different folders eventhough they're in the same package.
   */
  public void testJavaExtractorTwoSourceDirs() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    // Pick two sources in the same package that live in different directories.
    List<String> sources = Lists.newArrayList(PathUtil.join(TEST_DATA_DIR,  "one/pkg/A.java"),
        PathUtil.join(TEST_DATA_DIR, "two/pkg/B.java"));


    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());

    // With the expected sources as explicit sources.
    assertEquals(2, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactlyElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation for both sources.
    JavaArguments args = JavaArguments.parseArguments(unit);
    assertThat(args.getSourcepath())
        .containsExactly(PathUtil.join(TEST_DATA_DIR, "one"), PathUtil.join(TEST_DATA_DIR, "two"));
    assertThat(args.getClasspath()).isEmpty();
  }

  /**
   * Tests java compilation with classfiles in the classpath.
   * Test ensures that only used classfiles are stored.
   */
  public void testJavaExtractorClassFiles() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    // Index the specified sources
    List<String> sources = Lists.newArrayList(
        PathUtil.join(TEST_DATA_DIR, "child/derived/B.java"));

    String parentCp = PathUtil.join(TEST_DATA_DIR,  "parent/");
    List<String> classpath = Lists.newArrayList(parentCp);
    String classFile = PathUtil.join(parentCp, "base/A.class");

    // Index the specified sources and classes from the classpath
    CompilationDescription description =
        java.extract(TARGET1, sources, classpath, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());
    assertEquals(1, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0)).inOrder();

    // Ensure the right class files are picked up from the classpath.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactlyElementsIn(getExpectedInfos(
            Arrays.asList(sources.get(0), classFile)));

    JavaArguments args = JavaArguments.parseArguments(unit);
    assertThat(args.getSourcepath()).containsExactly(PathUtil.join(TEST_DATA_DIR, "child"));
    // Ensure the classpath is set for replay.
    assertThat(args.getClasspath()).containsExactly(parentCp);
  }

  /**
   * Tests java compilation with classfiles in a jarfile on the classpath.
   * Ensures that only used classfiles are stored.
   */
  public void testJavaExtractorJar() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    // Index the specified sources.
    List<String> sources = Lists.newArrayList(
        PathUtil.join(TEST_DATA_DIR, "child/derived/B.java"));

    String parentCp = PathUtil.join(TEST_DATA_DIR,  "jarred.jar");
    List<String> classpath = Lists.newArrayList(parentCp);

    // Index the specified sources and classes from inside jar on the classpath.
    CompilationDescription description =
        java.extract(TARGET1, sources, classpath, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertThat(unit).isNotNull();
    assertThat(unit.getVName().getSignature()).isEqualTo(TARGET1);
    assertThat(unit.getSourceFileList())
        .containsExactlyElementsIn(sources)
        .inOrder();

    // The classpath is adjusted to start wit !jar!
    String classFile = PathUtil.join(JavaCompilationUnitExtractor.JAR_ROOT, "base/A.class");

    // Ensure the right class files are picked up from the classpath from within the jar.
    assertThat(getInfos(unit.getRequiredInputList())).containsExactly(
        makeFileInfo(sources.get(0)),
        makeFileInfo(classFile, PathUtil.join(TEST_DATA_DIR, "parent/base/A.class")));

    JavaArguments args = JavaArguments.parseArguments(unit);
    assertEquals(1, args.getSourcepath().size());
    assertEquals(PathUtil.join(TEST_DATA_DIR, "child"), args.getSourcepath().get(0));
    assertEquals(1, args.getClasspath().size());
    // Ensure the magic !jar! classpath is added.
    assertEquals(JavaCompilationUnitExtractor.JAR_ROOT, args.getClasspath().get(0));
  }

  /**
   * Tests case where compile errors exist in the sources under compilation.
   * Compilation should still be created.
   */
  public void testJavaExtractorCompileError() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Lists.newArrayList(PathUtil.join(TEST_DATA_DIR, "/error/Crash.java"));

    // Index the specified sources, reporting compilation failure.
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());
    assertEquals(1, unit.getRequiredInputCount());
    assertEquals(sources.get(0), unit.getRequiredInput(0).getInfo().getPath());
    assertEquals(ExtractorUtils.digestForPath(sources.get(0)),
        unit.getRequiredInput(0).getInfo().getDigest());
    assertEquals(1, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0)).inOrder();

    JavaArguments args = JavaArguments.parseArguments(unit);
    assertEquals(1, args.getSourcepath().size());
    assertEquals(TEST_DATA_DIR, args.getSourcepath().get(0));
    assertEquals(0, args.getClasspath().size());
  }

  /**
   * Tests that dependent files are ordered correctly.
   */
  public void testJavaExtractorOrderedDependencies() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Lists.newArrayList(
        PathUtil.join(TEST_DATA_DIR,  "/deps/A.java"),
        PathUtil.join(TEST_DATA_DIR,  "/deps/C.java"),
        PathUtil.join(TEST_DATA_DIR, "/deps/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());
    assertEquals(3, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList())
        .containsExactly(sources.get(0), sources.get(1), sources.get(2))
        .inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactlyElementsIn(getExpectedInfos(
            Arrays.asList(sources.get(0), sources.get(2), sources.get(1))))
        .inOrder();
  }

  /**
   * Tests the case where one of the sources doesn't follow the java pattern of naming the path
   * to the source file after the package name.
   */
  public void testJavaExtractorNonConformingPath() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Lists.newArrayList(PathUtil.join(TEST_DATA_DIR,  "/path/sub/A.java"),
        PathUtil.join(TEST_DATA_DIR, "/path/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());
    assertEquals(2, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactlyElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaArguments args = JavaArguments.parseArguments(unit);
    assertEquals(1, args.getSourcepath().size());
    assertEquals(TEST_DATA_DIR, args.getSourcepath().get(0));
    assertEquals(0, args.getClasspath().size());
  }

  /**
   * Tests the case where one of the sources defines a package only type that is not conforming
   * to the classname matching the filename.
   */
  public void testJavaExtractorAdditionalTypeDefinitions() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = Lists.newArrayList(PathUtil.join(TEST_DATA_DIR, "/hitchhikers/A.java"),
        PathUtil.join(TEST_DATA_DIR, "/hitchhikers/B.java"));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());
    assertEquals(2, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0), sources.get(1)).inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactlyElementsIn(getExpectedInfos(sources));

    // And the correct sourcepath set to replay the compilation.
    JavaArguments args = JavaArguments.parseArguments(unit);
    assertThat(args.getSourcepath()).containsExactly(TEST_DATA_DIR);
    assertThat(args.getClasspath()).isEmpty();
  }

  /**
   * Tests that targets that contain annotation processors are indexed correctly.
   */
  public void testJavaExtractorAnnotationProcessing() throws Exception {
    String testDir = System.getenv("TEST_TMPDIR");
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS, testDir);

    File processorFile = Paths.get("kythe/javatests/com/google/devtools/kythe/extractors/java/SillyProcessor_deploy.jar").toFile();
    if (!processorFile.exists()) {
      throw new AssertionError("SillyProcessor_deploy.jar does not exist");
    }

    List<String> origSources = testFiles("processor/Silly.java", "processor/SillyUser.java");

    List<String> processorpath = Arrays.asList(processorFile.getPath());
    List<String> processors = Arrays.asList("processor.SillyProcessor");
    List<String> options = Arrays.asList("-s", new File(testDir, "output-gensrc.jar.files").getPath());

    new File(testDir, "output-gensrc.jar.files").mkdir();

    // Copy sources from runfiles into test dir
    List<String> testSources = new ArrayList<String>();
    for (String source : origSources) {
      Path destFile = Paths.get(testDir).resolve(source);
      Files.createDirectories(destFile.getParent());
      Files.copy(Paths.get(source), destFile, REPLACE_EXISTING);
      testSources.add(destFile.toString());
    }

    // Index the specified sources
    CompilationDescription description = java.extract(
        TARGET1, testSources, EMPTY, EMPTY, processorpath, processors, options, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());
    assertEquals(3, unit.getSourceFileCount());

    String sillyGenerated = "output-gensrc.jar.files/processor/SillyGenerated.java";
    assertThat(unit.getSourceFileList())
        .containsExactly(origSources.get(0), origSources.get(1), sillyGenerated)
        .inOrder();

    // With the expected sources as required inputs.
    assertThat(getInfos(unit.getRequiredInputList()))
        .containsExactly(
            makeFileInfo(origSources.get(0)),
            makeFileInfo(origSources.get(1)),
            makeFileInfo(sillyGenerated, Paths.get(testDir, sillyGenerated).toString()));

    // And the correct sourcepath set to replay the compilation.
    JavaArguments args = JavaArguments.parseArguments(unit);
    assertEquals(2, args.getSourcepath().size());
    assertThat(args.getSourcepath()).containsExactly(TEST_DATA_DIR, "output-gensrc.jar.files");
    assertEquals(0, args.getClasspath().size());
  }

  /**
   * Tests the case where one of the sources defines a package only type that is not conforming
   * to the classname matching the filename.
   */
  public void testEmptyCompilationUnit() throws Exception {
    JavaCompilationUnitExtractor java = new JavaCompilationUnitExtractor(CORPUS);

    List<String> sources = testFiles("empty/Empty.java");
    List<String> options = Lists.newArrayList(
        "-bootclasspath", testFiles("empty/fake-rt.jar").get(0));

    // Index the specified sources
    CompilationDescription description =
        java.extract(TARGET1, sources, EMPTY, EMPTY, EMPTY, EMPTY, options, "output");

    CompilationUnit unit = description.getCompilationUnit();
    assertNotNull(unit);
    assertEquals(TARGET1, unit.getVName().getSignature());
    assertEquals(1, unit.getSourceFileCount());
    assertThat(unit.getSourceFileList()).containsExactly(sources.get(0)).inOrder();

    assertEquals(2, unit.getRequiredInputCount());
    List<String> requiredPaths = new ArrayList<String>();
    for (FileInput input : unit.getRequiredInputList()) {
      requiredPaths.add(input.getInfo().getPath());
    }
    assertThat(requiredPaths).containsExactly(sources.get(0), "!jar!/java/lang/Fake.class");

    // And the correct classpath set to replay the compilation.
    JavaArguments args = JavaArguments.parseArguments(unit);
    assertEquals(1, args.getSourcepath().size());
    assertEquals(1, args.getClasspath().size());
    assertEquals("!jar!", args.getClasspath().get(0));
  }

  private List<String> testFiles(String... files) {
    List<String> res = new ArrayList<String>();
    for (String file : files) {
      res.add(PathUtil.join(TEST_DATA_DIR, file));
    }
    return res;
  }

  private static List<FileInfo> getInfos(List<FileInput> files) {
    return Lists.transform(files, new Function<FileInput, FileInfo>() {
          @Override
          public FileInfo apply(FileInput file) {
            return file.getInfo();
          }
        });
  }

  private static List<FileInfo> getExpectedInfos(List<String> files) {
    return Lists.transform(files, new Function<String, FileInfo>() {
          @Override
          public FileInfo apply(String file) {
            return makeFileInfo(file);
          }
        });
  }

  private static FileInfo makeFileInfo(String path) {
    try {
      return FileInfo.newBuilder()
          .setPath(path)
          .setDigest(ExtractorUtils.digestForPath(path))
          .build();
    } catch (Exception e) {
      throw Throwables.propagate(e);
    }
  }

  private static FileInfo makeFileInfo(String path, String localPath) {
    try {
      return FileInfo.newBuilder()
          .setPath(path)
          .setDigest(ExtractorUtils.digestForPath(localPath))
          .build();
    } catch (Exception e) {
      throw Throwables.propagate(e);
    }
  }

  private static class JavaArguments {
    private final List<String> sourcepath, classpath;

    private JavaArguments(List<String> sourcepath, List<String> classpath) {
      this.sourcepath = ImmutableList.copyOf(sourcepath);
      this.classpath = ImmutableList.copyOf(classpath);
    }

    public static JavaArguments parseArguments(CompilationUnit unit) {
      return parse(unit.getArgumentList());
    }

    public static JavaArguments parse(List<String> args) {
      List<String> sourcepath = Lists.newLinkedList();
      List<String> classpath = Lists.newLinkedList();
      for (int i = 0; i < args.size(); i++) {
        if (args.get(i).equals("-sourcepath")) {
          i++;
          sourcepath.addAll(parsePathList(args.get(i)));
          if (args.get(i).isEmpty()) {
            sourcepath.add(".");
          }
        } else if (args.get(i).equals("-cp") || args.get(i).equals("-classpath")) {
          i++;
          classpath.addAll(parsePathList(args.get(i)));
        }
      }
      return new JavaArguments(sourcepath, classpath);
    }

    public List<String> getSourcepath() {
      return sourcepath;
    }

    public List<String> getClasspath() {
      return classpath;
    }

    private static List<String> parsePathList(String paths) {
      if (paths.isEmpty()) {
        return EMPTY;
      }
      return ImmutableList.copyOf(paths.split(File.pathSeparator));
    }
  }
}
