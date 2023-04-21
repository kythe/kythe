/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Strings.isNullOrEmpty;
import static com.google.common.base.Strings.nullToEmpty;

import com.google.common.base.Joiner;
import com.google.common.base.Splitter;
import com.google.common.escape.Escaper;
import com.google.common.net.PercentEscaper;
import com.google.devtools.kythe.proto.CorpusPath;
import com.google.devtools.kythe.proto.Storage.VName;
import java.io.Serializable;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import org.checkerframework.checker.nullness.qual.Nullable;

/**
 * /{@link String} realization of a {@link VName}.
 *
 * <p>Specification: //kythe/util/go/kytheuri/kythe-uri-spec.txt
 */
public class KytheURI implements Serializable {
  private static final long serialVersionUID = 2726281203803801095L;

  public static final String SCHEME_LABEL = "kythe";
  public static final String SCHEME = SCHEME_LABEL + ":";
  public static final KytheURI EMPTY = new KytheURI();

  private static final String SAFE_CHARS = ".-_~";
  private static final Escaper PATH_ESCAPER = new PercentEscaper(SAFE_CHARS + "/", false);
  private static final Escaper ALL_ESCAPER = new PercentEscaper(SAFE_CHARS, false);
  private static final Splitter SIGNATURE_SPLITTER = Splitter.on('#');
  private static final Splitter.MapSplitter PARAMS_SPLITTER =
      Splitter.on('?').withKeyValueSeparator("=");
  private static final Splitter PATH_SPLITTER = Splitter.on('/');

  private final VName vName;

  /** Construct a new {@link KytheURI}. */
  public KytheURI(String signature, String corpus, String root, String path, String language) {
    this(
        VName.newBuilder()
            .setSignature(nullToEmpty(signature))
            .setCorpus(nullToEmpty(corpus))
            .setRoot(nullToEmpty(root))
            .setPath(nullToEmpty(path))
            .setLanguage(nullToEmpty(language))
            .build());
  }

  /** Constructs an empty {@link KytheURI}. */
  private KytheURI() {
    this(VName.getDefaultInstance());
  }

  /** Unpacks a {@link VName} into a new {@link KytheURI}. */
  public KytheURI(VName vName) {
    this.vName = vName;
  }

  /** Unpacks a {@link CorpusPath} into a new {@link KytheURI}. */
  public KytheURI(CorpusPath cp) {
    this(
        VName.newBuilder()
            .setCorpus(cp.getCorpus())
            .setRoot(cp.getRoot())
            .setPath(cp.getPath())
            .build());
  }

  /** Returns the {@link KytheURI}'s signature. */
  public String getSignature() {
    return vName.getSignature();
  }

  /** Returns the {@link KytheURI}'s corpus name. */
  public String getCorpus() {
    return vName.getCorpus();
  }

  /** Returns the {@link KytheURI}'s corpus path. */
  public String getPath() {
    return vName.getPath();
  }

  /** Returns the {@link KytheURI}'s corpus root. */
  public String getRoot() {
    return vName.getRoot();
  }

  /** Returns the {@link KytheURI}'s language. */
  public String getLanguage() {
    return vName.getLanguage();
  }

  /** Returns an equivalent {@link VName}. */
  public VName toVName() {
    return vName;
  }

  /** Returns the {@link CorpusPath} components of the uri. */
  public CorpusPath toCorpusPath() {
    return CorpusPath.newBuilder()
        .setCorpus(vName.getCorpus())
        .setRoot(vName.getRoot())
        .setPath(vName.getPath())
        .build();
  }

  @Override
  public String toString() {
    StringBuilder b =
        new StringBuilder(
            SCHEME.length()
                + "//?=?=?=#".length() // assume all punctuation is necessary
                + getCorpus().length()
                + getPath().length()
                + getRoot().length()
                + getLanguage().length()
                + getSignature().length());
    b.append(SCHEME);
    if (!vName.getCorpus().isEmpty()) {
      b.append("//");
      b.append(PATH_ESCAPER.escape(vName.getCorpus()));
    }
    attr(b, "lang", ALL_ESCAPER.escape(vName.getLanguage()));
    attr(b, "path", PATH_ESCAPER.escape(cleanPath(vName.getPath())));
    attr(b, "root", PATH_ESCAPER.escape(vName.getRoot()));
    if (!vName.getSignature().isEmpty()) {
      b.append("#");
      b.append(ALL_ESCAPER.escape(vName.getSignature()));
    }
    return b.toString();
  }

  @Override
  public int hashCode() {
    return vName.hashCode();
  }

  @Override
  public boolean equals(Object o) {
    return this == o || (o instanceof KytheURI && vName.equals(((KytheURI) o).vName));
  }

  /** Returns an equivalent Kythe ticket for the given {@link VName}. */
  public static String asString(VName vName) {
    return new KytheURI(vName).toString();
  }

  /** Returns an equivalent Kythe ticket for the given {@link CorpusPath}. */
  public static String asString(CorpusPath cp) {
    return new KytheURI(cp).toString();
  }

  /** Parses the given string to produce a new {@link KytheURI}. */
  public static KytheURI parse(String str) {
    checkNotNull(str, "str must be non-null");
    if (str.isEmpty() || str.equals(SCHEME) || str.equals(SCHEME + "//")) {
      return EMPTY;
    }
    String original = str;

    // Check for a scheme label.  This may be empty; but if present, it must be
    // our expected scheme.
    if (str.startsWith(SCHEME)) {
      str = str.substring(SCHEME.length());
    }

    String signature = null;
    String corpus = null;
    String path = null;
    String root = null;
    String lang = null;

    // Split corpus/attributes from signature.
    Iterator<String> parts = SIGNATURE_SPLITTER.split(str).iterator();
    String head = parts.next();
    if (parts.hasNext()) {
      signature = parts.next();
    }
    checkArgument(!parts.hasNext(), "URI has multiple fragments: %s", original);

    // Remove corpus prefix from attributes.
    int firstParam = head.indexOf('?');
    firstParam = firstParam < 0 ? head.length() : firstParam;
    if (head.startsWith("//")) {
      corpus = head.substring(2, firstParam);
      head = head.substring(firstParam);
    } else {
      checkArgument(firstParam == 0, "invalid URI scheme: %s", original);
    }

    // If there are any attributes, parse them.  We allow valid attributes to
    // occur in any order, even if it is not canonical.
    if (!head.isEmpty()) {
      Map<String, String> params = PARAMS_SPLITTER.split(head.substring(1));

      for (Map.Entry<String, String> e : params.entrySet()) {
        switch (e.getKey()) {
          case "path":
            path = e.getValue();
            break;
          case "root":
            root = e.getValue();
            break;
          case "lang":
            lang = e.getValue();
            break;
          default:
            checkArgument(false, "invalid attribute: %s (value: %s)", e.getKey(), e.getValue());
            break;
        }
      }
    }

    return new KytheURI(
        decode(signature), decode(corpus), decode(root), decode(path), decode(lang));
  }

  /** Parses the given Kythe ticket string to produce a new {@link VName}. */
  public static VName parseVName(String ticket) {
    return parse(ticket).toVName();
  }

  private static @Nullable String decode(String str) {
    if (isNullOrEmpty(str)) {
      return null;
    }
    try {
      return URLDecoder.decode(str.replace("+", "%2B"), "UTF-8");
    } catch (java.io.UnsupportedEncodingException e) {
      throw new RuntimeException(e);
    }
  }

  /** Returns a new {@link KytheURI.Builder}. */
  public static Builder newBuilder() {
    return new Builder();
  }

  public static class Builder {
    private final VName.Builder builder = VName.newBuilder();

    public Builder setSignature(String signature) {
      builder.setSignature(nullToEmpty(signature));
      return this;
    }

    public Builder setCorpus(String corpus) {
      builder.setCorpus(nullToEmpty(corpus));
      return this;
    }

    public Builder setPath(String path) {
      builder.setPath(nullToEmpty(path));
      return this;
    }

    public Builder setRoot(String root) {
      builder.setRoot(nullToEmpty(root));
      return this;
    }

    public Builder setLanguage(String language) {
      builder.setLanguage(nullToEmpty(language));
      return this;
    }

    public KytheURI build() {
      return new KytheURI(builder.build());
    }
  }

  private static void attr(StringBuilder b, String name, String value) {
    if (value.isEmpty()) {
      return;
    }
    b.append("?");
    b.append(name);
    b.append("=");
    b.append(value);
  }

  /**
   * Returns a lexically cleaned path, with repeated slashes and "." path components removed, and
   * ".." path components rewound as far as possible without touching the filesystem.
   */
  private static String cleanPath(String path) {
    ArrayList<String> clean = new ArrayList<>();
    if (!path.isEmpty() && path.charAt(0) == '/') {
      clean.add("");
    }
    for (String part : PATH_SPLITTER.split(path)) {
      if (part.isEmpty() || part.equals(".")) {
        continue; // skip empty path components and "here" markers.
      } else if (part.equals("..") && !clean.isEmpty()) {
        clean.remove(clean.size() - 1);
        continue; // back off if possible for "up" (..) markers.
      } else {
        clean.add(part);
      }
    }
    return Joiner.on('/').join(clean);
  }
}
