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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.Writer;
import java.net.URI;
import javax.lang.model.element.Modifier;
import javax.lang.model.element.NestingKind;
import javax.tools.JavaFileObject;

/**
 * Wraps JavaFileObjects and reports if the object is used as input by the compiler. We consider the
 * object used when the compiler asks for the data in the file. Javac will ask for all files in a
 * package, even if it already knows it needs a specific one. This is probably done for caching
 * optimization (especially reading from .jar files). In our case, less is better.
 */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
class UsageAsInputReportingJavaFileObject implements JavaFileObject {

  protected void markUsed() {
    usageRecord.markUsed();
  }

  protected JavaFileObject underlyingFileObject;
  protected InputUsageRecord usageRecord;

  public UsageAsInputReportingJavaFileObject(
      JavaFileObject underlyingFileObject, InputUsageRecord usageRecord) {
    this.underlyingFileObject = underlyingFileObject;
    this.usageRecord = usageRecord;
  }

  @Override
  public URI toUri() {
    return underlyingFileObject.toUri();
  }

  @Override
  public String getName() {
    return underlyingFileObject.getName();
  }

  @Override
  public InputStream openInputStream() throws IOException {
    usageRecord.markUsed();
    return underlyingFileObject.openInputStream();
  }

  @Override
  public OutputStream openOutputStream() throws IOException {
    return underlyingFileObject.openOutputStream();
  }

  @Override
  public Reader openReader(boolean ignoreEncodingErrors) throws IOException {
    usageRecord.markUsed();
    return underlyingFileObject.openReader(ignoreEncodingErrors);
  }

  @Override
  public CharSequence getCharContent(boolean ignoreEncodingErrors) throws IOException {
    usageRecord.markUsed();
    return underlyingFileObject.getCharContent(ignoreEncodingErrors);
  }

  @Override
  public Writer openWriter() throws IOException {
    return underlyingFileObject.openWriter();
  }

  @Override
  public long getLastModified() {
    return underlyingFileObject.getLastModified();
  }

  @Override
  public boolean delete() {
    return underlyingFileObject.delete();
  }

  @Override
  public Kind getKind() {
    return underlyingFileObject.getKind();
  }

  @Override
  public boolean isNameCompatible(String simpleName, Kind kind) {
    return underlyingFileObject.isNameCompatible(simpleName, kind);
  }

  @Override
  public NestingKind getNestingKind() {
    return underlyingFileObject.getNestingKind();
  }

  @Override
  public Modifier getAccessLevel() {
    return underlyingFileObject.getAccessLevel();
  }

  @Override
  public String toString() {
    return toUri().toString() + "@" + getKind().name();
  }
}
