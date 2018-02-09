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

package com.google.devtools.kythe.extractors.shared;

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.base.Preconditions;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.reflect.TypeToken;
import com.google.re2j.Matcher;
import com.google.re2j.Pattern;
import java.io.IOException;
import java.lang.reflect.Type;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.List;
import java.util.Stack;

/**
 * Utility for configuring base {@link VName}s to use for particular file paths. Useful for
 * populating the {@link VName}s for each required input in a {@link CompilationUnit}.
 *
 * JSON format:
 *   <pre>[
 *     {
 *       "pattern": "pathRegex",
 *       "vname": {
 *         "corpus": "corpusTemplate",  // optional
 *         "root": "rootTemplate"       // optional
 *       }
 *     }, ...
 *   ]</pre>
 *
 * The {@link VName} template values can contain markers such as @1@ or @2@ that will be replaced by
 * the first or second regex groups in the pathRegex.
 *
 * NOTE: regex syntax is RE2: https://code.google.com/p/re2/wiki/Syntax
 */
public class FileVNames {
  private static final Gson GSON =
      new GsonBuilder().registerTypeAdapter(Pattern.class, new PatternDeserializer()).create();
  private static final Type CONFIG_TYPE = new TypeToken<List<BaseFileVName>>() {}.getType();

  private final List<BaseFileVName> baseVNames;

  private FileVNames(List<BaseFileVName> baseVNames) {
    Preconditions.checkNotNull(baseVNames);
    for (BaseFileVName b : baseVNames) {
      Preconditions.checkNotNull(b.pattern, "pattern == null for base VName: %s", b.vname);
      Preconditions.checkNotNull(b.vname, "vname template == null for pattern: %s", b.pattern);
    }
    this.baseVNames = baseVNames;
  }

  /**
   * Returns a {@link FileVNames} that yields a static corpus-populated {@link VName} for each
   * {@link #lookupBaseVName(String)}.
   */
  public static FileVNames staticCorpus(String corpus) {
    return new FileVNames(
        Arrays.asList(
            new BaseFileVName(Pattern.compile(".*"), new VNameTemplate(corpus, null, null))));
  }

  public static FileVNames fromFile(String configFile) throws IOException {
    return fromFile(Paths.get(configFile));
  }

  public static FileVNames fromFile(Path configFile) throws IOException {
    return new FileVNames(
        GSON.<List<BaseFileVName>>fromJson(
            Files.newBufferedReader(configFile, UTF_8), CONFIG_TYPE));
  }

  public static FileVNames fromJson(String json) {
    return new FileVNames(GSON.<List<BaseFileVName>>fromJson(json, CONFIG_TYPE));
  }

  /**
   * Returns a base {@link VName} for the given file path. If none is configured, return {@link
   * VName#getDefaultInstance()}.
   */
  public VName lookupBaseVName(String path) {
    if (path != null) {
      for (BaseFileVName b : baseVNames) {
        Matcher matcher = b.pattern.matcher(path);
        if (matcher.matches()) {
          return b.vname.fillInWith(matcher);
        }
      }
    }
    return VName.getDefaultInstance();
  }

  /** Base {@link VName} to use for files matching {@code pattern}. */
  private static class BaseFileVName {
    public final Pattern pattern;
    public final VNameTemplate vname;

    public BaseFileVName(Pattern pattern, VNameTemplate vname) {
      this.pattern = pattern;
      this.vname = vname;
    }
  }

  /** Subset of a {@link VName} with built-in templating '@<num>@' markers. */
  private static class VNameTemplate {
    private final String corpus;
    private final String root;
    private final String path;

    public VNameTemplate(String corpus, String root, String path) {
      this.corpus = corpus;
      this.root = root;
      this.path = path;
    }

    /**
     * Returns a {@link VName} by filling in its corpus/root/path with regex groups in the given
     * {@link Matcher}.
     */
    public VName fillInWith(Matcher m) {
      VName.Builder b = VName.newBuilder();
      if (corpus != null) {
        b.setCorpus(fillIn(corpus, m));
      }
      if (root != null) {
        b.setRoot(fillIn(root, m));
      }
      if (path != null) {
        b.setPath(fillIn(path, m));
      }
      return b.build();
    }

    private static final Pattern replacerMatcher = Pattern.compile("@(\\d+)@");

    private static String fillIn(String tmpl, Matcher m) {
      Matcher replacers = replacerMatcher.matcher(tmpl);
      Stack<ReplacementMarker> matches = new Stack<>();
      while (replacers.find()) {
        matches.push(
            new ReplacementMarker(
                replacers.start(), replacers.end(), Integer.parseInt(replacers.group(1))));
      }
      StringBuilder builder = new StringBuilder(tmpl);
      while (!matches.isEmpty()) {
        ReplacementMarker res = matches.pop();
        builder.replace(res.start, res.end, m.group(res.grp));
      }
      return builder.toString();
    }

    @Override
    public String toString() {
      return String.format("{corpus: %s, root: %s, path: %s}", corpus, root, path);
    }
  }

  /** Representation of a '@n@' marker's span and integer parse of 'n'. */
  private static class ReplacementMarker {
    final int start;
    final int end;
    final int grp;

    ReplacementMarker(int start, int end, int grp) {
      this.start = start;
      this.end = end;
      this.grp = grp;
    }
  }

  private static class PatternDeserializer implements JsonDeserializer<Pattern> {
    @Override
    public Pattern deserialize(JsonElement json, Type typeOfT, JsonDeserializationContext context) {
      return Pattern.compile(json.getAsJsonPrimitive().getAsString());
    }
  }
}
