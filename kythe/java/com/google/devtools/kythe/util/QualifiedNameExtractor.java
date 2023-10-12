/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.util;

import com.google.devtools.kythe.doc.MarkedSourceRenderer;
import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.SymbolInfo;
import java.util.Optional;

/**
 * Provides a library of functions for extracting qualified names from java record/class marked
 * source nodes.
 */
public class QualifiedNameExtractor {
  private QualifiedNameExtractor() {}

  /**
   * Extracts a qualified name from the specified {@link MarkedSource} tree.
   *
   * @param markedSource The {@link MarkedSource} tree to be evaluated.
   * @return a {@link SymbolInfo} message containing the extracted name.
   */
  public static Optional<SymbolInfo> extractNameFromMarkedSource(MarkedSource markedSource) {
    String identifier = MarkedSourceRenderer.renderSimpleIdentifierText(markedSource);
    if (identifier.isEmpty()) {
      return Optional.empty();
    }
    String qualifiedName = MarkedSourceRenderer.renderSimpleQualifiedNameText(markedSource, true);
    SymbolInfo.Builder symbolInfo = SymbolInfo.newBuilder();
    symbolInfo.setBaseName(identifier);
    symbolInfo.setQualifiedName(qualifiedName);
    return Optional.of(symbolInfo.build());
  }
}
