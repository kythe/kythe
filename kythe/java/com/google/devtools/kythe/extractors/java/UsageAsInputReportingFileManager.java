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

package com.google.devtools.kythe.extractors.java;

import com.google.common.base.Function;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.tools.FileObject;
import javax.tools.ForwardingJavaFileManager;
import javax.tools.JavaFileObject;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardJavaFileManager;

/**
 * Wraps the StandardJavaFileManager to track which .java and .class files Javac touches for a given
 * compilation.
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
class UsageAsInputReportingFileManager extends ForwardingJavaFileManager<StandardJavaFileManager> {

  private final Map<URI, InputUsageRecord> inputUsageRecords = new HashMap<>();

  protected UsageAsInputReportingFileManager(StandardJavaFileManager fileManager) {
    super(fileManager);
  }

  /** Returns collection of JavaFileObjects that Javac read the contents of. */
  public Collection<JavaFileObject> getUsages() {
    Collection<JavaFileObject> result = new ArrayList<>();
    for (InputUsageRecord usageRecord : inputUsageRecords.values()) {
      if (usageRecord.isUsed()) {
        result.add(usageRecord.fileObject());
      }
    }
    return result;
  }

  @Override
  public String inferBinaryName(Location location, JavaFileObject file) {
    return fileManager.inferBinaryName(location, unwrap(file));
  }

  @Override
  public Iterable<JavaFileObject> list(
      Location location, String packageName, Set<Kind> kinds, boolean recurse) throws IOException {
    return Iterables.transform(
        fileManager.list(location, packageName, kinds, recurse),
        new Function<JavaFileObject, JavaFileObject>() {
          @Override
          public JavaFileObject apply(JavaFileObject input) {
            return map(input);
          }
        });
  }

  /** Wraps a JavaFileObject in a UsageAsInputReportingJavaFileObject, shares existing instances. */
  private JavaFileObject map(JavaFileObject item) {
    if (item == null) {
      return item;
    }
    InputUsageRecord usage = inputUsageRecords.get(item.toUri());
    if (usage == null) {
      usage = new InputUsageRecord(item);
      inputUsageRecords.put(item.toUri(), usage);
    }
    return new UsageAsInputReportingJavaFileObject(item, usage);
  }

  /** Helper to match loading source files and tracking their usage. */
  public Iterable<JavaFileObject> getJavaFileForSources(Iterable<String> sources) {
    List<String> sourceList = Lists.newArrayList(sources);
    String[] sourceArray = new String[sourceList.size()];
    sourceList.toArray(sourceArray);

    return Iterables.transform(
        fileManager.getJavaFileObjects(sourceArray),
        new Function<JavaFileObject, JavaFileObject>() {
          @Override
          public JavaFileObject apply(JavaFileObject input) {
            return map(input);
          }
        });
  }

  @Override
  public JavaFileObject getJavaFileForInput(
      final Location location, final String className, final Kind kind) throws IOException {
    return map(fileManager.getJavaFileForInput(location, className, kind));
  }

  @Override
  public JavaFileObject getJavaFileForOutput(
      Location location, String className, Kind kind, FileObject sibling) throws IOException {
    // A java file opened initially for output might later get reopened for input (e.g.,
    // source files generated during annotation processing), so we need to track them too.
    return map(fileManager.getJavaFileForOutput(location, className, kind, unwrap(sibling)));
  }

  @Override
  public FileObject getFileForOutput(
      Location location, String packageName, String relativeName, FileObject sibling)
      throws IOException {
    return fileManager.getFileForOutput(location, packageName, relativeName, unwrap(sibling));
  }

  @Override
  public boolean isSameFile(FileObject a, FileObject b) {
    return super.isSameFile(unwrap(a), unwrap(b));
  }

  // StandardJavaFileManager doesn't like it when it's asked about a JavaFileObject
  // it didn't create, so we need to unwrap our objects.
  private FileObject unwrap(FileObject jfo) {
    if (jfo instanceof UsageAsInputReportingJavaFileObject) {
      return ((UsageAsInputReportingJavaFileObject) jfo).underlyingFileObject;
    }
    return jfo;
  }

  private JavaFileObject unwrap(JavaFileObject jfo) {
    if (jfo instanceof UsageAsInputReportingJavaFileObject) {
      return ((UsageAsInputReportingJavaFileObject) jfo).underlyingFileObject;
    }
    return jfo;
  }
}
