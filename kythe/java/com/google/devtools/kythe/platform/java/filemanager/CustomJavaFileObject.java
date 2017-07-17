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

import com.google.devtools.kythe.platform.shared.FileDataProvider;
import java.nio.charset.Charset;
import javax.lang.model.element.Modifier;
import javax.lang.model.element.NestingKind;
import javax.tools.JavaFileObject;

/** JavaFileObject that can be provided a body at a future time. */
@com.sun.tools.javac.api.ClientCodeWrapper.Trusted
public class CustomJavaFileObject extends CustomFileObject implements JavaFileObject {

  private final String className;
  private Kind kind;

  public CustomJavaFileObject(
      FileDataProvider contentProvider,
      String path,
      String digest,
      String className,
      Kind kind,
      Charset encoding) {
    super(contentProvider, path, digest, encoding);
    this.className = className;
    this.kind = kind;
  }

  @Override
  public Kind getKind() {
    return kind;
  }

  @Override
  public boolean isNameCompatible(String simpleName, Kind kind) {
    return getName().equals(simpleName + kind.extension);
  }

  @Override
  public NestingKind getNestingKind() {
    return null;
  }

  @Override
  public Modifier getAccessLevel() {
    return null;
  }

  public String getClassName() {
    return className;
  }

  @Override
  public String toString() {
    return "CustomJavaFileObject [className=" + className + ", kind=" + kind + "]";
  }
}
