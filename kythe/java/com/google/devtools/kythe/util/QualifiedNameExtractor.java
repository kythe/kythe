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
   * @return The resolved qualified name if found.
   */
  public static Optional<String> extractNameFromMarkedSource(MarkedSource markedSource) {
    // extract the class name
    Optional<MarkedSource> identifier =
        retrieveFirstMarkedSourceByKind(
            markedSource, MarkedSource.Kind.IDENTIFIER, /* isPretextRequired */ true);
    if (!identifier.isPresent()) {
      // if the current class node's marked source does not contain an identifier, skip it
      return Optional.empty();
    }
    String className = identifier.get().getPreText();

    // extract the class's package name
    Optional<MarkedSource> context =
        retrieveFirstMarkedSourceByKind(
            markedSource, MarkedSource.Kind.CONTEXT, /* isPretextRequired */ false);
    if (!context.isPresent()) {
      // if the current class node's marked source does not contain a context, skip it
      return Optional.empty();
    }
    String packageDelim =
        !context.get().getPostChildText().isEmpty() ? context.get().getPostChildText() : ".";
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

    // emit the qualified class name
    return packageName.isEmpty()
        ? Optional.of(className)
        : Optional.of(String.format("%s%s%s", packageName, packageDelim, className));
  }

  /**
   * Traverse the specified {@link MarkedSource} tree, returning the first node that matches the
   * specified kind.
   *
   * @param markedSource The root of the {@link MarkedSource} tree to be traversed.
   * @param kind The kind of the requested node.
   * @param isPretextRequired Whether or not the existence of a pretext string is required.
   */
  private static Optional<MarkedSource> retrieveFirstMarkedSourceByKind(
      MarkedSource markedSource, MarkedSource.Kind kind, boolean isPretextRequired) {
    Optional<MarkedSource> result =
        markedSource
            .getChildList()
            .stream()
            .filter(
                child ->
                    child.getKind().equals(kind)
                        && (isPretextRequired ? !child.getPreText().isEmpty() : true))
            .findFirst();
    if (result.isPresent()) {
      return result;
    }

    for (MarkedSource child : markedSource.getChildList()) {
      result = retrieveFirstMarkedSourceByKind(child, kind, isPretextRequired);
      if (result.isPresent()) {
        return result;
      }
    }

    return Optional.empty();
  }
}
