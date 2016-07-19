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

package com.google.devtools.kythe.util;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Strings.emptyToNull;
import static com.google.common.base.Strings.isNullOrEmpty;
import static com.google.common.base.Strings.nullToEmpty;

import com.google.common.base.Joiner;
import com.google.devtools.kythe.proto.Storage.VName;
import java.io.Serializable;
import java.net.URI;
import java.net.URISyntaxException;

/**
 * {@link URI}/{@link String} realization of a {@link VName}.
 *
 * Specification: //kythe/util/go/kytheuri/kythe-uri-spec.txt
 */
public class KytheURI implements Serializable {
  private static final long serialVersionUID = 2726281203803801095L;

  public static final String SCHEME_LABEL = "kythe";
  public static final KytheURI EMPTY = new KytheURI();

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

  /**
   * Returns an equivalent {@link URI}.
   *
   * @throw URISyntaxException when the corpus/path/root/language are all empty
   */
  public URI toURI() throws URISyntaxException {
    String query =
        Joiner.on("?")
            .skipNulls()
            .join(
                attr("lang", vName.getLanguage()),
                attr("path", vName.getPath()),
                attr("root", vName.getRoot()));
    String corpus = vName.getCorpus();
    String authority = corpus, path = null;
    int slash = corpus.indexOf('/');
    if (slash != -1) {
      authority = corpus.substring(0, slash);
      path = corpus.substring(slash);
    }
    return new URI(
            SCHEME_LABEL,
            emptyToNull(authority),
            path,
            emptyToNull(query),
            emptyToNull(vName.getSignature()))
        .normalize();
  }

  /** Returns an equivalent {@link VName}. */
  public VName toVName() {
    return vName;
  }

  @Override
  public String toString() {
    if (vName.getCorpus().isEmpty()
        && vName.getPath().isEmpty()
        && vName.getRoot().isEmpty()
        && vName.getLanguage().isEmpty()) {
      // java.net.URI does not handle an empty scheme-specific-part well...
      return vName.getSignature().isEmpty()
          ? SCHEME_LABEL + ":"
          : SCHEME_LABEL + ":#" + vName.getSignature();
    }
    try {
      return toURI().toString().replace("+", "%2B");
    } catch (URISyntaxException e) {
      throw new IllegalArgumentException("URI failed to construct", e);
    }
  }

  @Override
  public int hashCode() {
    return vName.hashCode();
  }

  @Override
  public boolean equals(Object o) {
    return this == o || (o instanceof KytheURI && vName.equals(((KytheURI) o).vName));
  }

  /** Parses the given string to produce a new {@link KytheURI}. */
  public static KytheURI parse(String str) throws URISyntaxException {
    checkNotNull(str, "str must be non-null");

    // java.net.URI does not handle an empty scheme-specific-part well...
    str = str.replaceFirst("^" + SCHEME_LABEL + ":([^/]|$)", SCHEME_LABEL + "://$1");
    if (str.isEmpty() || str.equals(SCHEME_LABEL + "://")) {
      return EMPTY;
    }

    URI uri = new URI(str).normalize();
    checkArgument(
        SCHEME_LABEL.equals(uri.getScheme()) || isNullOrEmpty(uri.getScheme()),
        "URI Scheme must be " + SCHEME_LABEL + "; was " + uri.getScheme());
    String root = null, path = null, lang = null;
    if (!isNullOrEmpty(uri.getQuery())) {
      for (String attr : uri.getQuery().split("\\?")) {
        String[] keyValue = attr.split("=", 2);
        if (keyValue.length != 2) {
          throw new URISyntaxException(str, "Invalid query: " + uri.getQuery());
        }
        switch (keyValue[0]) {
          case "lang":
            lang = keyValue[1];
            break;
          case "root":
            root = keyValue[1];
            break;
          case "path":
            path = keyValue[1];
            break;
          default:
            throw new URISyntaxException(str, "Invalid query attribute: " + keyValue[0]);
        }
      }
    }
    String signature = uri.getFragment();
    String corpus =
        ((uri.getHost() == null ? nullToEmpty(uri.getAuthority()) : nullToEmpty(uri.getHost()))
                + nullToEmpty(uri.getPath()))
            .replaceAll("/+$", "");
    return new KytheURI(signature, corpus, root, path, lang);
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

  private static String attr(String name, String value) {
    return value.isEmpty() ? null : name + "=" + value;
  }
}
