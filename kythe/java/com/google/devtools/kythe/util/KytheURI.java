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
import java.util.Objects;

/**
 * {@link URI}/{@link String} realization of a {@link VName}.
 *
 * Specification: //kythe/util/go/kytheuri/kythe-uri-spec.txt
 */
public class KytheURI implements Serializable {
  private static final long serialVersionUID = 2726281203803801095L;

  public static final String SCHEME_LABEL = "kythe";
  public static final KytheURI EMPTY = new KytheURI();

  private final String signature, corpus, root, path, language;

  /** Construct a new {@link KytheURI}. */
  public KytheURI(String signature, String corpus, String root, String path, String language) {
    this.signature = emptyToNull(signature);
    this.corpus = emptyToNull(corpus);
    this.root = emptyToNull(root);
    this.path = emptyToNull(path);
    this.language = emptyToNull(language);
  }

  /** Constructs an empty {@link KytheURI}. */
  private KytheURI() {
    this(null, null, null, null, null);
  }

  /** Unpacks a {@link VName} into a new {@link KytheURI}. */
  public KytheURI(VName vname) {
    this(vname.getSignature(),
        vname.getCorpus(),
        vname.getRoot(),
        vname.getPath(),
        vname.getLanguage());
  }

  /** Returns the {@link KytheURI}'s signature. */
  public String getSignature() {
    return signature;
  }

  /** Returns the {@link KytheURI}'s corpus name. */
  public String getCorpus() {
    return corpus;
  }

  /** Returns the {@link KytheURI}'s corpus path. */
  public String getPath() {
    return path;
  }

  /** Returns the {@link KytheURI}'s corpus root. */
  public String getRoot() {
    return root;
  }

  /** Returns the {@link KytheURI}'s language. */
  public String getLanguage() {
    return language;
  }

  /**
   * Returns an equivalent {@link URI}.
   *
   * @throw URISyntaxException when the corpus/path/root/language are all empty
   */
  public URI toURI() throws URISyntaxException {
    String query = Joiner.on("?").skipNulls()
        .join(attr("lang", language), attr("path", path), attr("root", root));
    String authority = corpus, path = null;
    int slash = corpus != null ? corpus.indexOf('/') : -1;
    if (slash != -1) {
      authority = corpus.substring(0, slash);
      path = corpus.substring(slash);
    }
    return new URI(SCHEME_LABEL,
        authority, path, emptyToNull(query), signature).normalize();
  }

  /** Returns an equivalent {@link VName}. */
  public VName toVName() {
    VName.Builder builder = VName.newBuilder();
    if (signature != null) {
      builder.setSignature(signature);
    }
    if (corpus != null) {
      builder.setCorpus(corpus);
    }
    if (path != null) {
      builder.setPath(path);
    }
    if (root != null) {
      builder.setRoot(root);
    }
    if (language != null) {
      builder.setLanguage(language);
    }
    return builder.build();
  }

  @Override
  public String toString() {
    if (corpus == null && path == null && root == null && language == null) {
      // java.net.URI does not handle an empty scheme-specific-part well...
      return signature == null
          ? SCHEME_LABEL + ":"
          : SCHEME_LABEL + ":#" + signature;
    }
    try {
      return toURI().toString().replace("+", "%2B");
    } catch (URISyntaxException e) {
      throw new IllegalArgumentException("URI failed to construct", e);
    }
  }

  @Override
  public int hashCode() {
    return Objects.hash(signature, corpus, language, path, root);
  }

  @Override
  public boolean equals(Object o) {
    return this == o ||
        (o instanceof KytheURI && toString().equals(o.toString()));
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
    checkArgument(SCHEME_LABEL.equals(uri.getScheme()) || isNullOrEmpty(uri.getScheme()),
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
    String corpus = ((uri.getHost() == null
            ? nullToEmpty(uri.getAuthority())
            : nullToEmpty(uri.getHost()))
        + nullToEmpty(uri.getPath())).replaceAll("/+$", "");
    return new KytheURI(signature, corpus, root, path, lang);
  }

  /** Returns a new {@link KytheURI.Builder}. */
  public static Builder newBuilder() {
    return new Builder();
  }

  public static class Builder {
    private String signature, corpus, language, path, root;

    public Builder setSignature(String signature) {
      this.signature = signature;
      return this;
    }

    public Builder setCorpus(String corpus) {
      this.corpus = corpus;
      return this;
    }

    public Builder setPath(String path) {
      this.path = path;
      return this;
    }

    public Builder setRoot(String root) {
      this.root = root;
      return this;
    }

    public Builder setLanguage(String language) {
      this.language = language;
      return this;
    }

    public KytheURI build() {
      return new KytheURI(signature, corpus, root, path, language);
    }
  }

  private static String attr(String name, String value) {
    return value == null
        ? null
        : name + "=" + value;
  }
}
