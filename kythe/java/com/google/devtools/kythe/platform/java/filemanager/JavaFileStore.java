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

import java.util.Set;
import javax.tools.JavaFileObject.Kind;

/**
 * JavaFileStore is a service for finding Java compiler related files such as {@code .class} or
 * {@code .java}, or potentially any file that can be consumed by Java compiler. It is up to the
 * implementors of this interface to define what is backing their store. The store can be supplied
 * by a single jar file, multiple jar files, a database table, a directory on disk or any
 * combination of these.
 *
 * <p>File store is not tied to a specific Java path location. Meaning a store can be used for class
 * or source retrieval. Users of this service can differentiate between the two using a path prefix.
 */
public interface JavaFileStore {

  /**
   * Finds and returns the Java file object associated with the {@code className} and {@code kind}.
   * {@code pathPrefixes} are used to limit possible search paths. There is no limitation on the
   * format of a path. It is up to implementation to decide what it should be.
   *
   * @param className
   * @param kind
   * @param pathPrefixes
   * @return the found Java file object as {@link CustomJavaFileObject} or null if no match is found
   */
  public CustomJavaFileObject find(String className, Kind kind, Set<String> pathPrefixes);

  /**
   * Finds and returns the file object in a package, {@code packageName}, with name {@code
   * relativeName}. {@code pathPrefixes} are used to limit possible search paths. There is no
   * limitation on the format of a path. It is up to implementation to decide what it should be.
   *
   * @param packageName
   * @param relativeName
   * @param pathPrefixes
   * @return the found Java file object as {@link CustomFileObject} or null if no match is found
   */
  public CustomFileObject find(String packageName, String relativeName, Set<String> pathPrefixes);

  /**
   * Finds and returns the Java file object with the exact path matching {@code path} and {@code
   * kind}. There is no limitation on the format of a path. It is up to implementation to decide
   * what it should be.
   *
   * @param path
   * @param kind
   * @return the found Java file object as {@link CustomJavaFileObject} or null if no match is found
   */
  public CustomJavaFileObject findByPath(String path, Kind kind);

  /**
   * Finds and returns all the Java file objects in a package, {@code packageName}. {@code
   * pathPrefixes} are used to limit possible search paths. There is no limitation on the format of
   * a path. It is up to implementation to decide what it should be.
   *
   * @param packageName
   * @param kinds
   * @param pathPrefixes
   * @param recurse
   * @return all the found Java file objects or an empty set if none can be found
   */
  public Set<CustomJavaFileObject> list(
      String packageName, Set<Kind> kinds, Set<String> pathPrefixes, boolean recurse);
}
