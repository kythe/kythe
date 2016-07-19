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

import com.google.devtools.kythe.common.FormattingLogger;
import com.sun.tools.javac.code.Symbol.TypeSymbol;
import java.io.IOException;
import java.util.Set;
import javax.annotation.processing.AbstractProcessor;
import javax.annotation.processing.RoundEnvironment;
import javax.annotation.processing.SupportedAnnotationTypes;
import javax.lang.model.SourceVersion;
import javax.lang.model.element.TypeElement;
import javax.tools.JavaFileObject.Kind;
import javax.tools.StandardLocation;

/**
 * This class is used to visit all the annotation used in Java files and record the usage of these
 * annotation as JavaFileObject. This processing will eliminate some of the platform errors caused
 * by javac not being able to find the class for some of the annotations in the analysis phase. The
 * recorded files will later be put in the datastore for analysis phase.
 */
@SupportedAnnotationTypes(value = {"*"})
public class ProcessAnnotation extends AbstractProcessor {

  private static final FormattingLogger logger =
      FormattingLogger.getLogger(ProcessAnnotation.class);

  UsageAsInputReportingFileManager fileManager;

  public ProcessAnnotation(UsageAsInputReportingFileManager fileManager) {
    this.fileManager = fileManager;
  }

  @Override
  public boolean process(Set<? extends TypeElement> annotations, RoundEnvironment roundEnv) {
    for (TypeElement e : annotations) {
      TypeSymbol s = (TypeSymbol) e;
      try {
        UsageAsInputReportingJavaFileObject jfo =
            (UsageAsInputReportingJavaFileObject)
                fileManager.getJavaFileForInput(
                    StandardLocation.CLASS_OUTPUT, s.flatName().toString(), Kind.CLASS);
        if (jfo == null) {
          jfo =
              (UsageAsInputReportingJavaFileObject)
                  fileManager.getJavaFileForInput(
                      StandardLocation.CLASS_PATH, s.flatName().toString(), Kind.CLASS);
        }
        if (jfo == null) {
          jfo =
              (UsageAsInputReportingJavaFileObject)
                  fileManager.getJavaFileForInput(
                      StandardLocation.SOURCE_PATH, s.flatName().toString(), Kind.CLASS);
        }
        if (jfo != null) {
          jfo.markUsed();
        }
      } catch (IOException ex) {
        // We only log any IO exception here and do not cancel the whole processing because of an
        // exception in this stage.
        logger.severefmt("Error in annotation processing: %s", ex.getMessage());
      }
    }
    // We must return false so normal processors run after us.
    return false;
  }

  @Override
  public SourceVersion getSupportedSourceVersion() {
    return SourceVersion.latest();
  }
}
