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

package com.google.devtools.kythe.analyzers.java;

import com.google.common.collect.Lists;
import com.google.devtools.kythe.platform.java.helpers.SyntaxPreservingScanner;
import com.google.devtools.kythe.util.Span;

import com.sun.tools.javac.parser.Tokens.Token;
import com.sun.tools.javac.parser.Tokens.TokenKind;
import com.sun.tools.javac.tree.EndPosTable;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Name;

import java.io.IOException;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.tools.JavaFileObject;

/** Utility class to provide ANCHOR positions in Java sources. */
public final class FilePositions {
  private final JavaFileObject sourceFile;
  private final EndPosTable endPositions;
  private final Map<Name, List<Span>> identTable = new HashMap<>();

  private final String text;
  private final Charset encoding;

  public FilePositions(Context context, JCCompilationUnit compilation, Charset sourceEncoding)
      throws IOException {
    sourceFile = compilation.getSourceFile();
    endPositions = compilation.endPositions;
    // Assume content has been decoded correctly by kythe.platform.java.filemanager.CustomFileObject
    text = sourceFile.getCharContent(true).toString();
    encoding = sourceEncoding;

    // Filling up the identifier lookup table:
    SyntaxPreservingScanner scanner = SyntaxPreservingScanner.create(context, text);
    for (Token token = scanner.readToken(); token.kind != TokenKind.EOF;
        token = scanner.readToken()) {
      if (token.kind == TokenKind.IDENTIFIER) {
        addIdentifier(token.name(), scanner.spanForToken(token));
      }
    }
  }

  public JavaFileObject getSourceFile() {
    return sourceFile;
  }

  public String getFilename() {
    return sourceFile.getName();
  }

  public Charset getEncoding() {
    return encoding;
  }

  public String getSourceText() {
    return text;
  }

  public byte[] getData() {
    return getSourceText().getBytes(encoding);
  }

  /**
   * Returns the {@link Span} for the first known occurrence of the specified {@link Name}d
   * identifier, starting at or after the specified starting offset.  Returns {@code null} if no
   * occurrences are found.
   */
  public Span findIdentifier(Name name, int startOffset) {
    List<Span> spans = identTable.get(name);
    if (spans != null) {
      startOffset = charToByteOffset(startOffset);
      for (Span span : spans) {
        if (span.getStart() >= startOffset) {
          return span;
        }
      }
    }
    return null;
  }

  /**
   * Returns the starting byte offset for the given tree in the source text. If {@code tree} is
   * {@code null} or no position is known, -1 is returned.
   */
  public int getStart(JCTree tree) {
    return charToByteOffset(tree.getStartPosition());
  }

  /**
   * Returns the ending byte offset for the given tree in the source text. If {@code tree} is
   * {@code null} or no position is known, -1 is returned.
   */
  public int getEnd(JCTree tree) {
    // TODO(schroederc): properly handle -1 positions
    return charToByteOffset(tree.getEndPosition(endPositions));
  }

  private int charToByteOffset(int charOffset) {
    if (charOffset < 0) {
      return -1;
    }
    // TODO(schroederc): determine if this is a memory bottle-neck
    return text.substring(0, charOffset).getBytes(encoding).length;
  }

  // Adds an identifier location to the lookup table for findIdentifier().
  private void addIdentifier(Name name, Span position) {
    List<Span> spans = identTable.get(name);
    if (spans == null) {
      spans = Lists.newArrayList();
      identTable.put(name, spans);
    }
    spans.add(new Span(charToByteOffset(position.getStart()), charToByteOffset(position.getEnd())));
  }
}
