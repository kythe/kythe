/*
 * Copyright 2017 Google Inc. All rights reserved.
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

import com.google.devtools.kythe.proto.MarkedSource;
import com.google.devtools.kythe.proto.SymbolInfo;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Provides a library of functions for extracting qualified names from java record/class marked
 * source nodes.
 */
public class QualifiedNameExtractor {
  /**
   * Extracts a qualified name from the specified {@link MarkedSource} tree.
   *
   * @param markedSource The {@link MarkedSource} tree to be evaluated.
   * @return a {@link SymbolInfo} message containing the extracted name.
   */
  public static Optional<SymbolInfo> extractNameFromMarkedSource(MarkedSource markedSource) {
    // Extract the base name. Note that the base name may be at the root, so
    // check for that case explicitly.
    Optional<MarkedSource> identifier;
    if (markedSourceMatches(
        markedSource, MarkedSource.Kind.IDENTIFIER, /* isPretextRequired */ true)) {
      identifier = Optional.of(markedSource);
    } else {
      identifier =
          retrieveFirstMarkedSourceByKind(
              markedSource, MarkedSource.Kind.IDENTIFIER, /* isPretextRequired */ true);
    }
    if (!identifier.isPresent()) {
      // if the current class node's marked source does not contain an identifier, skip it
      return Optional.empty();
    }
    String baseName = identifier.get().getPreText();

    // extract the class's package name
    String qualifiedName = null;
    Optional<MarkedSource> context =
        retrieveFirstMarkedSourceByKind(
            markedSource, MarkedSource.Kind.CONTEXT, /* isPretextRequired */ false);
    if (context.isPresent()) {
      String postChildText = context.get().getPostChildText();
      String packageDelim = !postChildText.isEmpty() ? postChildText : ".";
      String packageName =
          String.join(
              packageDelim,
              context
                  .get()
                  .getChildList()
                  .stream()
                  .filter(
                      child ->
                          child.getKind().equals(MarkedSource.Kind.IDENTIFIER)
                              && !child.getPreText().isEmpty())
                  .map(MarkedSource::getPreText)
                  .collect(Collectors.toList()));

      if (!packageName.isEmpty()) {
        qualifiedName = String.format("%s%s%s", packageName, postChildText, baseName);
      }
    }

    // emit the symbol info
    SymbolInfo.Builder symbolInfo = SymbolInfo.newBuilder();
    symbolInfo.setBaseName(baseName);
    if (qualifiedName != null) {
      symbolInfo.setQualifiedName(qualifiedName);
    }

    return Optional.of(symbolInfo.build());
  }

  /**
   * Traverse the specified {@link MarkedSource} tree breadth-first, returning the first node that
   * matches the specified kind.
   *
   * @param markedSource The root of the {@link MarkedSource} tree to be traversed.
   * @param kind The kind of the requested node.
   * @param isPretextRequired Whether or not the existence of a pretext string is required.
   */
  private static Optional<MarkedSource> retrieveFirstMarkedSourceByKind(
      MarkedSource markedSource, MarkedSource.Kind kind, boolean isPretextRequired) {
    // Check children explicitly before recurring; a match at this level should
    // be preferred to a deeper match (breadth-first).
    for (MarkedSource child : markedSource.getChildList()) {
      if (markedSourceMatches(child, kind, isPretextRequired)) {
        return Optional.of(child);
      }
    }

    // Recur into children.
    for (MarkedSource child : markedSource.getChildList()) {
      Optional<MarkedSource> result =
          retrieveFirstMarkedSourceByKind(child, kind, isPretextRequired);
      if (result.isPresent()) {
        return result;
      }
    }

    return Optional.empty();
  }

  /** Report whether markedSource has the specified kind and required pre-text. */
  private static boolean markedSourceMatches(
      MarkedSource markedSource, MarkedSource.Kind kind, boolean isPretextRequired) {
    return markedSource.getKind().equals(kind)
        && (!isPretextRequired || !markedSource.getPreText().isEmpty());
  }
}
