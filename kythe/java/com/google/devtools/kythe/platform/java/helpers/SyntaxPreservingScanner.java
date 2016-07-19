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

package com.google.devtools.kythe.platform.java.helpers;

import com.google.devtools.kythe.util.Span;
import com.sun.tools.javac.parser.JavaTokenizer;
import com.sun.tools.javac.parser.ScannerFactory;
import com.sun.tools.javac.parser.Tokens;
import com.sun.tools.javac.parser.Tokens.Comment.CommentStyle;
import com.sun.tools.javac.parser.Tokens.Token;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Position;
import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Extends the javac Scanner to support comments. Comments are injected into the token stream as
 * CUSTOM tokens; the consumer is responsible for identifying them and processing their contents.
 */
public class SyntaxPreservingScanner extends JavaTokenizer {
  public List<CustomToken> customTokens = new ArrayList<>();
  private final Position.LineMap lineMap;

  /** An abstract class to represent the data for a custom token */
  public abstract static class CustomToken {
    public Span span;
    public String text;

    public abstract boolean isComment();

    public CustomToken(Span span, String text) {
      super();
      this.span = span;
      this.text = text;
    }
  }

  /** A comment token */
  public static class CommentToken extends CustomToken {
    public CommentStyle style;

    public CommentToken(Span span, String text, CommentStyle style) {
      super(span, text);
      this.style = style;
    }

    @Override
    public boolean isComment() {
      return true;
    }
  }

  /** Creates a new scanner for the given compiler context and source text. */
  public static SyntaxPreservingScanner create(Context context, CharSequence input) {
    ScannerFactory factory = ScannerFactory.instance(context);
    if (input instanceof CharBuffer) {
      return new SyntaxPreservingScanner(factory, (CharBuffer) input);
    } else {
      char[] array = input.toString().toCharArray();
      return new SyntaxPreservingScanner(factory, array);
    }
  }

  @Override
  protected Tokens.Comment processComment(int pos, int endPos, CommentStyle style) {
    customTokens.add(
        new CommentToken(
            new Span(pos, endPos), new String(reader.getRawCharacters(pos, endPos)), style));
    return super.processComment(pos, endPos, style);
  }

  @Override
  public Position.LineMap getLineMap() {
    return this.lineMap;
  }

  /**
   * Returns a {@link Span} message corresponding to the current token, or null if there is no
   * current token.
   */
  public Span spanForToken(Token token) {
    return new Span(token.pos, token.endPos);
  }

  private SyntaxPreservingScanner(ScannerFactory factory, CharBuffer input) {
    super(factory, input);
    this.lineMap = super.getLineMap();
  }

  private SyntaxPreservingScanner(ScannerFactory factory, char[] input) {
    super(factory, input, input.length);
    this.lineMap = super.getLineMap();
  }
}
