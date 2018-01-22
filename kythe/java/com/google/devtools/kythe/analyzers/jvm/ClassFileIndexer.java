/*
 * Copyright 2018 Google Inc. All rights reserved.
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

package com.google.devtools.kythe.analyzers.jvm;

import com.google.devtools.kythe.analyzers.base.FactEmitter;
import com.google.devtools.kythe.analyzers.base.StreamFactEmitter;
import com.google.devtools.kythe.extractors.shared.CompilationDescription;
import com.google.devtools.kythe.extractors.shared.IndexInfoUtils;
import com.google.devtools.kythe.platform.shared.NullStatisticsCollector;
import com.google.devtools.kythe.platform.shared.StatisticsCollector;
import com.google.devtools.kythe.proto.Analysis.FileData;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;

/**
 * Kythe analyzer for JVM class files (possibly within a jar or kindex file).
 *
 * <p>Usage: class_file_indexer <class_file | jar_file | kindex_file> ...
 */
public class ClassFileIndexer {
  public static void main(String[] args) throws IOException {
    try (OutputStream stream = System.out) {
      FactEmitter emitter = new StreamFactEmitter(stream);
      StatisticsCollector statistics = NullStatisticsCollector.getInstance();
      KytheClassVisitor classVisitor = new KytheClassVisitor(statistics, emitter);
      for (String fileName : args) {
        File file = new File(fileName);
        if (fileName.endsWith(".jar")) {
          visitJarClassFiles(file, classVisitor);
        } else if (fileName.endsWith(".class")) {
          visitClassFile(file, classVisitor);
        } else if (fileName.endsWith(".kindex")) {
          CompilationDescription desc = IndexInfoUtils.readIndexInfoFromFile(fileName);
          for (FileData data : desc.getFileContents()) {
            if (data.getInfo().getPath().endsWith(".class")) {
              visitClassFile(data.getContent().newInput(), classVisitor);
            }
          }
        } else {
          throw new IllegalArgumentException("unknown file path extension: " + fileName);
        }
      }
    }
  }

  private static void visitJarClassFiles(File jarFile, ClassVisitor visitor) throws IOException {
    try (JarFile jar = new JarFile(jarFile)) {
      for (Enumeration<JarEntry> entries = jar.entries(); entries.hasMoreElements(); ) {
        JarEntry entry = entries.nextElement();
        if (!entry.getName().endsWith(".class")) {
          continue;
        }
        try (InputStream input = jar.getInputStream(entry)) {
          visitClassFile(input, visitor);
        }
      }
    }
  }

  private static void visitClassFile(File classFile, ClassVisitor visitor) throws IOException {
    try (InputStream input = new FileInputStream(classFile)) {
      visitClassFile(input, visitor);
    }
  }

  private static void visitClassFile(InputStream classFile, ClassVisitor visitor)
      throws IOException {
    new ClassReader(classFile).accept(visitor, 0);
  }
}
