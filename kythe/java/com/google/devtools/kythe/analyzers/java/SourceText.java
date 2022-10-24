/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

import com.google.devtools.kythe.platform.java.helpers.SyntaxPreservingScanner;
import com.google.devtools.kythe.platform.java.helpers.SyntaxPreservingScanner.CommentToken;
import com.google.devtools.kythe.util.PositionMappings;
import com.google.devtools.kythe.util.Span;
import com.sun.tools.javac.parser.Tokens.Token;
import com.sun.tools.javac.parser.Tokens.TokenKind;
import com.sun.tools.javac.tree.EndPosTable;
import com.sun.tools.javac.tree.JCTree;
import com.sun.tools.javac.tree.JCTree.JCCompilationUnit;
import com.sun.tools.javac.util.Context;
import java.io.IOException;
import java.nio.charset.Charset;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.NavigableSet;
import java.util.SortedSet;
import java.util.TreeSet;
import javax.lang.model.element.Name;
import javax.tools.JavaFileObject;
import org.checkerframework.checker.nullness.qual.Nullable;

public final class SourceText {
  private final Positions positions;
  private final List<Comment> comments = new ArrayList<>();

  public SourceText(Context context, JCCompilationUnit compilation, Charset sourceEncoding)
      throws IOException {
    JavaFileObject sourceFile = compilation.getSourceFile();
    // Assume content has been decoded correctly by kythe.platform.java.filemanager.CustomFileObject
    CharSequence text = sourceFile.getCharContent(true);

    positions = new Positions(sourceFile, compilation.endPositions, text, sourceEncoding);

    // Filling up the positions identifier lookup table
    SyntaxPreservingScanner scanner = SyntaxPreservingScanner.create(context, text);
    Deque<Token> starts = new ArrayDeque<>();
    for (Token token = scanner.readToken();
        token.kind != TokenKind.EOF;
        token = scanner.readToken()) {
      if (token.kind == TokenKind.IDENTIFIER) {
        positions.addIdentifier(token.name(), scanner.spanForToken(token));
      } else if (token.kind == TokenKind.NEW) {
        positions.addIdentifier(Keyword.NEW, scanner.spanForToken(token));
      } else if (token.kind == TokenKind.CLASS) {
        positions.addIdentifier(Keyword.CLASS, scanner.spanForToken(token));
      } else if (token.kind == TokenKind.LT) {
        starts.addFirst(token);
      } else if (token.kind == TokenKind.GT) {
        if (!starts.isEmpty()) {
          int start = scanner.spanForToken(starts.removeFirst()).getStart();
          int end = scanner.spanForToken(token).getEnd();
          positions.addBracketGroup(new Span(start, end));
        }
      }
    }

    for (SyntaxPreservingScanner.CustomToken token : scanner.customTokens) {
      if (!token.isComment()) {
        continue;
      }
      comments.add(new Comment(positions, (CommentToken) token));
    }
  }

  public Positions getPositions() {
    return positions;
  }

  public List<Comment> getComments() {
    return comments;
  }

  public static final class Comment {
    public final Span charSpan;
    public final Span lineSpan;
    public final Span byteSpan;
    public final String text;

    private Comment(Positions pos, CommentToken token) {
      text = token.text;
      charSpan = token.span;
      lineSpan =
          new Span(pos.charToLine(token.span.getStart()), pos.charToLine(token.span.getEnd()));
      byteSpan =
          new Span(
              pos.charToByteOffset(token.span.getStart()),
              pos.charToByteOffset(token.span.getEnd()));
    }
  }

  /** Names for keywords that must act as anchors. */
  public static enum Keyword implements Name {
    NEW("new"),
    CLASS("class");

    private final String keyword;

    private Keyword(String keyword) {
      this.keyword = keyword;
    }

    @Override
    public boolean contentEquals(CharSequence cs) {
      return keyword.contentEquals(cs);
    }

    @Override
    public char charAt(int index) {
      return keyword.charAt(index);
    }

    @Override
    public int length() {
      return keyword.length();
    }

    @Override
    public CharSequence subSequence(int start, int end) {
      return keyword.subSequence(start, end);
    }

    @Override
    public String toString() {
      return keyword;
    }
  }

  /** Utility class to provide ANCHOR positions in Java sources. */
  public static final class Positions {
    private final JavaFileObject sourceFile;
    private final EndPosTable endPositions;
    private final Map<Name, List<Span>> identTable = new HashMap<>();
    private final NavigableSet<Span> bracketGroups = new TreeSet<>();

    private final CharSequence text;
    private final PositionMappings mappings;
    private final Charset encoding;

    private Positions(
        JavaFileObject sourceFile, EndPosTable endPositions, CharSequence text, Charset encoding) {
      this.sourceFile = sourceFile;
      this.endPositions = endPositions;
      this.text = text;
      this.encoding = encoding;
      this.mappings = new PositionMappings(encoding, text);
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
      return text.toString();
    }

    public byte[] getData() {
      return getSourceText().getBytes(encoding);
    }

    /**
     * Returns the {@link Span} for the first known occurrence of the specified {@link Name}d
     * identifier, starting at or after the specified starting offset. Returns {@code null} if no
     * occurrences are found.
     */
    public @Nullable Span findIdentifier(Name name, int startOffset) {
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

    @SuppressWarnings("JdkObsolete")
    public @Nullable Span findBracketGroup(int startCharOffset) {
      int startOffset = charToByteOffset(startCharOffset);
      SortedSet<Span> grps = bracketGroups.tailSet(new Span(startOffset, startOffset));
      return grps.isEmpty() ? null : grps.first();
    }

    /** Returns the byte span for the given tree in the source text. */
    public Span getSpan(JCTree tree) {
      int start = getStart(tree);
      int end = getEnd(tree);
      return start >= 0 && end == -1 ? new Span(start, start) : new Span(start, end);
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

    // Adds an identifier location to the lookup table for findIdentifier().
    private void addIdentifier(Name name, Span position) {
      List<Span> spans = identTable.computeIfAbsent(name, k -> new ArrayList<>());
      spans.add(
          new Span(charToByteOffset(position.getStart()), charToByteOffset(position.getEnd())));
    }

    // Adds a bracket group for findBracketGroup lookups.
    private void addBracketGroup(Span bracketGroup) {
      bracketGroups.add(bracketGroup);
    }

    int charToLine(int charOffset) {
      return mappings.charToLine(charOffset);
    }

    int charToByteOffset(int charOffset) {
      return mappings.charToByteOffset(charOffset);
    }
  }
}
