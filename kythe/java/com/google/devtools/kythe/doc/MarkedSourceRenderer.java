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

package com.google.devtools.kythe.doc;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Sets;
import com.google.common.html.types.SafeHtml;
import com.google.common.html.types.SafeHtmlBuilder;
import com.google.common.html.types.SafeUrl;
import com.google.devtools.kythe.proto.Link;
import com.google.devtools.kythe.proto.MarkedSource;
import java.util.function.Function;
import java.util.stream.Stream;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Renders MarkedSource messages into user-printable {@link SafeHtml}. */
public class MarkedSourceRenderer {
  private MarkedSourceRenderer() {}

  /** Don't recurse more than this many times when rendering MarkedSource. */
  private static final int MAX_RENDER_DEPTH = 10;

  /**
   * Render {@code signature} as a full signature.
   *
   * @param makeLink if provided, this function will be used to generate link URIs from semantic
   *     node tickets. It may return null if there is no available URI.
   * @param signature the {@link MarkedSource} to render.
   */
  public static SafeHtml renderSignature(
      Function<String, SafeUrl> makeLink, MarkedSource signature) {
    return new RenderSimpleIdentifierState(makeLink)
        .render(
            signature,
            Sets.immutableEnumSet(
                MarkedSource.Kind.IDENTIFIER, MarkedSource.Kind.TYPE, MarkedSource.Kind.PARAMETER),
            0);
  }

  /**
   * Render {@code signature} a full signature as plaintext.
   *
   * @param signature the {@link MarkedSource} to render.
   */
  public static String renderSignatureText(MarkedSource signature) {
    return new RenderSimpleIdentifierState(null)
        .renderText(
            signature,
            Sets.immutableEnumSet(
                MarkedSource.Kind.IDENTIFIER,
                MarkedSource.Kind.TYPE,
                MarkedSource.Kind.PARAMETER,
                MarkedSource.Kind.MODIFIER),
            0);
  }

  /**
   * Extract and render a plaintext initializer from {@code signature}.
   *
   * @param makeLink if provided, this function will be used to generate link URIs from semantic
   *     node tickets. It may return null if there is no available URI.
   * @param signature the {@link MarkedSource} to render from.
   * @return SafeHtml.EMPTY if there is no such initializer.
   */
  public static SafeHtml renderInitializer(
      Function<String, SafeUrl> makeLink, MarkedSource signature) {
    return new RenderSimpleIdentifierState(makeLink)
        .render(signature, Sets.immutableEnumSet(MarkedSource.Kind.INITIALIZER), 0);
  }

  /**
   * Extract and render the simple qualified name from {@code signature}.
   *
   * @param makeLink if provided, this function will be used to generate link URIs from semantic
   *     node tickets. It may return null if there is no available URI.
   * @param signature the {@link MarkedSource} to render from.
   * @param includeIdentifier if true, include the identifier on the qualified name.
   * @return SafeHtml.EMPTY if there is no such initializer.
   */
  public static SafeHtml renderSimpleQualifiedName(
      Function<String, SafeUrl> makeLink, MarkedSource signature, boolean includeIdentifier) {
    return new RenderSimpleIdentifierState(makeLink)
        .render(
            signature,
            includeIdentifier
                ? Sets.immutableEnumSet(MarkedSource.Kind.IDENTIFIER, MarkedSource.Kind.CONTEXT)
                : Sets.immutableEnumSet(MarkedSource.Kind.CONTEXT),
            0);
  }

  /**
   * Extract and render the simple qualified name from {@code signature} as plaintext.
   *
   * @param signature the {@link MarkedSource} to render from.
   * @param includeIdentifier if true, include the identifier on the qualified name.
   * @return "" if there is no such name.
   */
  public static String renderSimpleQualifiedNameText(
      MarkedSource signature, boolean includeIdentifier) {
    return new RenderSimpleIdentifierState(null)
        .renderText(
            signature,
            includeIdentifier
                ? Sets.immutableEnumSet(MarkedSource.Kind.IDENTIFIER, MarkedSource.Kind.CONTEXT)
                : Sets.immutableEnumSet(MarkedSource.Kind.CONTEXT),
            0);
  }

  /**
   * Extract and render a the simple identifier from {@code signature}.
   *
   * @param makeLink if provided, this function will be used to generate link URIs from semantic
   *     node tickets. It may return null if there is no available URI.
   * @param signature the {@link MarkedSource} to render from.
   * @return SafeHtml.EMPTY if there is no such identifier.
   */
  public static SafeHtml renderSimpleIdentifier(
      Function<String, SafeUrl> makeLink, MarkedSource signature) {
    return new RenderSimpleIdentifierState(makeLink)
        .render(signature, Sets.immutableEnumSet(MarkedSource.Kind.IDENTIFIER), 0);
  }

  /**
   * Extract and render a the simple identifier from {@code signature} as plaintext.
   *
   * @param signature the {@link MarkedSource} to render from.
   * @return "" if there is no such identifier.
   */
  public static String renderSimpleIdentifierText(MarkedSource signature) {
    return new RenderSimpleIdentifierState(null)
        .renderText(signature, Sets.immutableEnumSet(MarkedSource.Kind.IDENTIFIER), 0);
  }

  /**
   * Extract and render the simple identifiers for parameters in {@code signature}.
   *
   * @param makeLink if provided, this function will be used to generate link URIs from semantic
   *     node tickets. It may return null if there is no available URI.
   * @param signature the {@link MarkedSource} to render from.
   * @return SafeHtml.EMPTY for those parameters without simple identifiers.
   */
  public static ImmutableList<SafeHtml> renderSimpleParams(
      Function<String, SafeUrl> makeLink, MarkedSource signature) {
    ImmutableList.Builder<SafeHtml> builder = ImmutableList.builder();
    RenderSimpleIdentifierState.renderSimpleParams(makeLink, signature, builder, 0);
    return builder.build();
  }

  private static class RenderSimpleIdentifierState {
    RenderSimpleIdentifierState(Function<String, SafeUrl> makeLink) {
      this.makeLink = makeLink;
    }

    public SafeHtml render(MarkedSource node, ImmutableSet<MarkedSource.Kind> enabled, int level) {
      htmlBuffer = new SafeHtmlBuilder("span");
      textBuffer = null;
      renderChild(node, enabled, ImmutableSet.of(), level);
      return bufferIsNonempty ? htmlBuffer.build() : SafeHtml.EMPTY;
    }

    public String renderText(
        MarkedSource node, ImmutableSet<MarkedSource.Kind> enabled, int level) {
      textBuffer = new StringBuilder();
      htmlBuffer = null;
      renderChild(node, enabled, ImmutableSet.of(), level);
      return bufferIsNonempty ? textBuffer.toString() : "";
    }

    private static void renderSimpleParams(
        Function<String, SafeUrl> makeLink,
        MarkedSource node,
        ImmutableList.Builder<SafeHtml> out,
        int level) {
      if (level >= MAX_RENDER_DEPTH) {
        return;
      }
      switch (node.getKind()) {
        case BOX:
          for (MarkedSource child : node.getChildList()) {
            renderSimpleParams(makeLink, child, out, level + 1);
          }
          break;
        case PARAMETER:
          for (MarkedSource child : node.getChildList()) {
            out.add(
                new RenderSimpleIdentifierState(makeLink)
                    .render(child, Sets.immutableEnumSet(MarkedSource.Kind.IDENTIFIER), level + 1));
          }
          break;
        default:
          break;
      }
    }

    private static boolean willRender(
        MarkedSource node, ImmutableSet<MarkedSource.Kind> enabled, int level) {
      MarkedSource.Kind kind = node.getKind();
      return level < MAX_RENDER_DEPTH
          && (kind.equals(MarkedSource.Kind.BOX)
              || kind.equals(MarkedSource.Kind.IDENTIFIER)
              || enabled.contains(kind));
    }

    private void renderChild(
        MarkedSource node,
        ImmutableSet<MarkedSource.Kind> enabled,
        ImmutableSet<MarkedSource.Kind> under,
        int level) {
      if (level >= MAX_RENDER_DEPTH) {
        return;
      }
      MarkedSource.Kind kind = node.getKind();
      if (!MarkedSource.Kind.BOX.equals(kind)) {
        if (!MarkedSource.Kind.IDENTIFIER.equals(kind) && !enabled.contains(kind)) {
          return;
        }
        under = Stream.concat(under.stream(), Stream.of(kind)).collect(Sets.toImmutableEnumSet());
      }
      SafeHtmlBuilder savedBuilder = null;
      if (shouldRender(enabled, under)) {
        SafeUrl link = makeLinkForSource(node);
        if (link != null && htmlBuffer != null) {
          savedBuilder = htmlBuffer;
          htmlBuffer = new SafeHtmlBuilder("a").setHref(link);
        }
        append(node.getPreText());
      }
      int lastRenderedChild = -1;
      for (int child = 0; child < node.getChildCount(); ++child) {
        if (willRender(node.getChild(child), enabled, level + 1)) {
          lastRenderedChild = child;
        }
      }
      for (int child = 0; child < node.getChildCount(); ++child) {
        MarkedSource c = node.getChild(child);
        if (willRender(c, enabled, level + 1)) {
          renderChild(c, enabled, under, level + 1);
          if (lastRenderedChild > child) {
            append(node.getPostChildText());
          } else if (node.getAddFinalListToken()) {
            appendFinalListToken(node.getPostChildText());
          }
        }
      }
      if (shouldRender(enabled, under)) {
        append(node.getPostText());
        if (savedBuilder != null) {
          htmlBuffer = savedBuilder.appendContent(htmlBuffer.build());
        }
        if (node.getKind() == MarkedSource.Kind.TYPE) {
          appendHeuristicSpace();
        }
      }
    }

    private boolean shouldRender(
        ImmutableSet<MarkedSource.Kind> enabled, ImmutableSet<MarkedSource.Kind> under) {
      for (MarkedSource.Kind kind : enabled) {
        if (under.contains(kind)) {
          return true;
        }
      }
      return false;
    }

    private @Nullable SafeUrl makeLinkForSource(MarkedSource node) {
      if (makeLink == null) {
        return null;
      }
      for (Link link : node.getLinkList()) {
        for (String definition : link.getDefinitionList()) {
          SafeUrl safeLink = makeLink.apply(definition);
          if (safeLink != null) {
            return safeLink;
          }
        }
      }
      return null;
    }

    private void append(String text) {
      if (prependBuffer != null && prependBuffer.length() > 0 && !text.isEmpty()) {
        String prependString = prependBuffer.toString();
        if (htmlBuffer != null) {
          htmlBuffer = htmlBuffer.escapeAndAppendContent(prependString);
        } else {
          textBuffer.append(prependString);
        }
        bufferEndsInSpace = prependString.endsWith(" ");
        prependBuffer.setLength(0);
        bufferIsNonempty = true;
      }
      if (htmlBuffer != null) {
        htmlBuffer = htmlBuffer.escapeAndAppendContent(text);
      } else {
        textBuffer.append(text);
      }
      if (!text.isEmpty()) {
        bufferEndsInSpace = text.endsWith(" ");
        bufferIsNonempty = true;
      }
    }

    /**
     * Escapes and adds {@code text} before the (non-empty) text that would be added by the next
     * call to {@link append}.
     */
    private void appendFinalListToken(String text) {
      if (prependBuffer == null) {
        prependBuffer = new StringBuilder();
      }
      prependBuffer.append(text);
    }

    /**
     * Make sure there's a space between the current content of the buffer and whatever is appended
     * to it later on.
     */
    private void appendHeuristicSpace() {
      if (bufferIsNonempty && !bufferEndsInSpace) {
        appendFinalListToken(" ");
      }
    }

    /** The buffer used to hold escaped HTML data. */
    private SafeHtmlBuilder htmlBuffer;

    /** The buffer used to hold text data. */
    private StringBuilder textBuffer;

    /** True if the last character in buffer is a space. */
    private boolean bufferEndsInSpace;

    /** True if the buffer is non-empty. */
    private boolean bufferIsNonempty;

    /**
     * Unescaped text that should be escaped and appended before any other text is appended to the
     * buffer.
     */
    private StringBuilder prependBuffer;

    private Function<String, SafeUrl> makeLink;
  }
}
