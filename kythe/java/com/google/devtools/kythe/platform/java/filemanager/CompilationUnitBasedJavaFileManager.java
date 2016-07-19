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

package com.google.devtools.kythe.platform.java.filemanager;

import com.google.devtools.kythe.extractors.java.JavaCompilationUnitExtractor;
import com.google.devtools.kythe.platform.shared.FileDataProvider;
import com.google.devtools.kythe.proto.Analysis.CompilationUnit;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.protobuf.Any;
import com.google.protobuf.InvalidProtocolBufferException;
import java.nio.charset.Charset;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import javax.tools.StandardJavaFileManager;
import javax.tools.StandardLocation;

/**
 * Makes it easier for our analysis to provide files in different ways than on the local file system
 * e.g. from datastores or .index files. All users of this class have to do is provide a {@link
 * FileDataProvider} that will feed the actual content in as a future.
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public class CompilationUnitBasedJavaFileManager extends JavaFileStoreBasedFileManager {

  /**
   * paths searched for .class files, can be relative or absolute, but must match path as named by
   * the extractor.
   */
  private final Set<String> classpath = new HashSet<>();

  /**
   * paths searched for .java files, can be relative or absolute, but must match path as named by
   * the extractor.
   */
  private final Set<String> sourcepath = new HashSet<>();

  public CompilationUnitBasedJavaFileManager(
      FileDataProvider contentProvider,
      CompilationUnit unit,
      StandardJavaFileManager fileManager,
      Charset encoding) {
    super(new CompilationUnitBasedJavaFileStore(unit, contentProvider, encoding), fileManager);

    // TODO(schroederc): determine if we want to keep around legacy argument parsing support
    classpath.add("");
    classpath.addAll(getPathSet(unit.getArgumentList(), "-cp"));
    classpath.addAll(getPathSet(unit.getArgumentList(), "-classpath"));
    sourcepath.add("");
    sourcepath.addAll(getPathSet(unit.getArgumentList(), "-sourcepath"));

    JavaDetails details = getDetails(unit);
    if (details != null) {
      classpath.addAll(details.getClasspathList());
      sourcepath.addAll(details.getSourcepathList());
    }
  }

  @Override
  protected Set<String> getSearchPaths(Location location) {
    Set<String> dirsToLookIn = new HashSet<>();
    if (location == StandardLocation.CLASS_PATH) {
      dirsToLookIn = classpath;
    } else if (location == StandardLocation.SOURCE_PATH) {
      dirsToLookIn = sourcepath;
    }
    return dirsToLookIn;
  }

  private static Set<String> getPathSet(List<String> options, String optName) {
    for (int i = 0; i < options.size(); i++) {
      if (options.get(i).equals(optName)) {
        if (i + 1 >= options.size()) {
          throw new IllegalArgumentException("Malformed " + optName + " argument");
        }
        Set<String> paths = new HashSet<>();
        for (String path : options.get(i + 1).split(":")) {
          paths.add(path);
        }
        return paths;
      }
    }
    return new HashSet<String>();
  }

  private static JavaDetails getDetails(CompilationUnit unit) {
    for (Any details : unit.getDetailsList()) {
      if (details.getTypeUrl().equals(JavaCompilationUnitExtractor.JAVA_DETAILS_URL)) {
        try {
          return JavaDetails.parseFrom(details.getValue());
        } catch (InvalidProtocolBufferException ipbe) {
          System.err.println(
              "WARNING: "
                  + "CompilationUnit contains JavaDetails that could not be parsed: "
                  + ipbe);
        }
      }
    }
    return null;
  }
}
