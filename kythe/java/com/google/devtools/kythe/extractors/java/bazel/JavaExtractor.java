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

package com.google.devtools.kythe.extractors.java.bazel;

import static com.google.common.base.StandardSystemProperty.USER_DIR;
import static com.google.common.io.Files.touch;
import static java.util.stream.Collectors.toCollection;

import com.google.common.base.Optional;
import com.google.common.collect.Lists;
import com.google.common.flogger.FluentLogger;
import com.google.common.io.ByteSource;
import com.google.common.io.MoreFiles;
import com.google.devtools.build.lib.actions.extra.ExtraActionInfo;
import com.google.devtools.build.lib.actions.extra.ExtraActionsBase;
import com.google.devtools.build.lib.actions.extra.JavaCompileInfo;
import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.ExtractionException;
import com.google.devtools.kythe.extractors.shared.FileVNames;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.protobuf.CodedInputStream;
import com.google.protobuf.ExtensionRegistry;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/** Java CompilationUnit extractor using Bazel's extra_action feature. */
public class JavaExtractor {

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  public static void main(String[] args) throws IOException, ExtractionException {
    JsonUtil.usingTypeRegistry(JsonUtil.JSON_TYPE_REGISTRY);

    if (args.length != 3) {
      System.err.println("Usage: java_extractor extra-action-file output-file vname-config");
      System.exit(1);
    }

    String extraActionPath = args[0];
    String outputPath = args[1];
    String vNamesConfigPath = args[2];

    ExtensionRegistry registry = ExtensionRegistry.newInstance();
    ExtraActionsBase.registerAllExtensions(registry);

    ExtraActionInfo info;
    try (InputStream stream = Files.newInputStream(Paths.get(extraActionPath))) {
      CodedInputStream coded = CodedInputStream.newInstance(stream);
      info = ExtraActionInfo.parseFrom(coded, registry);
    }

    if (!info.hasExtension(JavaCompileInfo.javaCompileInfo)) {
      throw new IllegalArgumentException("Given ExtraActionInfo without JavaCompileInfo");
    }

    JavaCompileInfo jInfo = info.getExtension(JavaCompileInfo.javaCompileInfo);

    List<String> sources = Lists.newArrayList(jInfo.getSourceFileList());
    List<String> sourcepaths = jInfo.getSourcepathList();

    if (!sourcepaths.isEmpty()) {
      List<String> updatedSourcepaths = new ArrayList<>();
      for (String sourcepath : sourcepaths) {
        // Support source jars like proto compilation outputs.
        if (sourcepath.endsWith(".jar") || sourcepath.endsWith(".srcjar")) {
          extractSourceJar(sourcepath, sources);
        } else {
          updatedSourcepaths.add(sourcepath);
        }
      }
      sourcepaths = updatedSourcepaths;
    }

    if (sources.isEmpty()) {
      // Skip binary-only compilations; there is nothing to analyze.
      touch(new File(outputPath));
      return;
    }

    List<String> javacOpts =
        jInfo.getJavacOptList().stream()
            .filter(
                // Filter out Bazel-specific flags.  Bazel adds its own flags (such as error-prone
                // flags) to the javac_opt list that cannot be handled by the standard javac
                // compiler, or in turn, by this extractor.
                opt ->
                    !(opt.startsWith("-Werror:")
                        || opt.startsWith("-extra_checks")
                        || opt.startsWith("-Xep")))
            .collect(toCollection(ArrayList::new));

    // Set up a fresh output directory
    javacOpts.add("-d");
    Path output = Files.createTempDirectory("output");
    javacOpts.add(output.toString());

    // Add the generated sources directory if any processors could be invoked.
    Optional<Path> genSrcDir = Optional.absent();
    if (!jInfo.getProcessorList().isEmpty()) {
      try {
        genSrcDir = readGeneratedSourceDirParam(jInfo);
      } catch (IOException ioe) {
        logger.atWarning().withCause(ioe).log(
            "Failed to find generated sources directory from javac parameters");
      }
      if (!genSrcDir.isPresent()) {
        genSrcDir = Optional.of(Files.createTempDirectory("sourcegendir"));
      }
      javacOpts.add("-s");
      javacOpts.add(genSrcDir.get().toString());
      // javac expects the directory to already exist.
      Files.createDirectories(genSrcDir.get());
    }

    CompilationDescription description =
        new JavaCompilationUnitExtractor(FileVNames.fromFile(vNamesConfigPath), USER_DIR.value())
            .extract(
                info.getOwner(),
                sources,
                jInfo.getClasspathList(),
                jInfo.getBootclasspathList(),
                sourcepaths,
                jInfo.getProcessorpathList(),
                jInfo.getProcessorList(),
                genSrcDir,
                javacOpts,
                jInfo.getOutputjar());

    if (outputPath.endsWith(IndexInfoUtils.KZIP_FILE_EXT)) {
      IndexInfoUtils.writeKzipToFile(description, outputPath);
    } else {
      IndexInfoUtils.writeKindexToFile(description, outputPath);
    }
  }

  /** Extracts a source jar and adds all java files in it to the list of sources. */
  private static void extractSourceJar(String sourcepath, List<String> sources) throws IOException {
    // We unzip the sourcefiles to a temp location (<the location of the jar>.files/)
    File tempFile = new File(sourcepath + ".files");
    if (tempFile.exists()) {
      MoreFiles.deleteRecursively(tempFile.toPath());
    }
    tempFile.mkdirs();
    List<String> files = unzipFile(new ZipFile(sourcepath), tempFile);

    // And update the list of sources based on the .java files we unzipped.
    files.stream().filter(input -> input.endsWith(".java")).forEachOrdered(sources::add);
  }

  /** Unzips specified zipFile to targetDirectory and returns a list of the unzipped files. */
  private static List<String> unzipFile(final ZipFile zipFile, File targetDirectory)
      throws IOException {
    List<String> files = new ArrayList<>();
    try {
      Enumeration<? extends ZipEntry> entries = zipFile.entries();
      // Zip Slip fix courtesy of snyk.io/research/zip-slip-vulnerability.
      String canonicalDirPath = targetDirectory.getCanonicalPath() + File.separator;
      while (entries.hasMoreElements()) {
        final ZipEntry entry = entries.nextElement();
        File targetFile = new File(targetDirectory, entry.getName());
        String canonicalFilePath = targetFile.getCanonicalPath();
        if (!canonicalFilePath.startsWith(canonicalDirPath)) {
          throw new IllegalArgumentException(
              "Zip archive trying to write file outside of target dir: " + canonicalFilePath);
        }
        if (entry.isDirectory()) {
          if (!targetFile.isDirectory() && !targetFile.mkdirs()) {
            throw new IOException("Failed to create directory: " + targetFile.getAbsolutePath());
          }
        } else {
          File parentFile = targetFile.getParentFile();
          if (!parentFile.isDirectory()) {
            if (!parentFile.mkdirs()) {
              throw new IOException("Failed to create directory: " + parentFile.getAbsolutePath());
            }
          }
          // Write the file to the destination.
          new ByteSource() {
            @Override
            public InputStream openStream() throws IOException {
              return zipFile.getInputStream(entry);
            }
          }.copyTo(MoreFiles.asByteSink(targetFile.toPath()));
          files.add(targetFile.getAbsolutePath());
        }
      }
    } finally {
      zipFile.close();
    }
    return files;
  }

  private static final String SOURCEGENDIR_FLAG = "--sourcegendir";

  /** Reads Bazel's compilation parameters and returns the value of the --sourcegendir flag. */
  private static Optional<Path> readGeneratedSourceDirParam(JavaCompileInfo jInfo)
      throws IOException {
    for (int i = 0; i < jInfo.getArgumentCount() - 1; i++) {
      if (jInfo.getArgument(i).equals(SOURCEGENDIR_FLAG)) {
        return Optional.of(Paths.get(jInfo.getArgument(i + 1)));
      }
    }

    // Fall-back to reading from the Bazel .params file
    try (java.io.BufferedReader params =
        Files.newBufferedReader(
            Paths.get(jInfo.getOutputjar() + "-2.params"),
            java.nio.charset.StandardCharsets.UTF_8)) {
      String line;
      while ((line = params.readLine()) != null) {
        if (SOURCEGENDIR_FLAG.equals(line)) {
          return Optional.of(Paths.get(params.readLine()));
        }
      }
      return Optional.absent();
    } catch (NoSuchFileException nsfe) {
      // params file is not guaranteed to exist; convert to missing path
      return Optional.absent();
    }
  }
}
