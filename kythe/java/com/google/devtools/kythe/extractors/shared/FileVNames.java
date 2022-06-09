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

package com.google.devtools.kythe.extractors.shared;

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.google.common.base.Suppliers;
import com.google.common.collect.Streams;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.proto.Storage.VNameRewriteRule;
import com.google.devtools.kythe.proto.Storage.VNameRewriteRules;
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
import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Deque;
import java.util.List;
import java.util.function.Supplier;
import java.util.stream.Collectors;

/**
 * Utility for configuring base {@link VName}s to use for particular file paths. Useful for
 * populating the {@link VName}s for each required input in a {@link CompilationUnit}.
 *
 * <p>JSON format:
 *
 * <pre>[
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
 * <p>NOTE: regex syntax is RE2: https://github.com/google/re2/wiki/Syntax
 */
public class FileVNames {
  private static final Gson GSON =
      new GsonBuilder().registerTypeAdapter(Pattern.class, new PatternDeserializer()).create();
  private static final Type CONFIG_TYPE = new TypeToken<List<BaseFileVName>>() {}.getType();

  private final List<BaseFileVName> baseVNames;
  private final Supplier<String> defaultCorpus;

  private FileVNames(List<BaseFileVName> baseVNames) {
    Preconditions.checkNotNull(baseVNames);
    for (BaseFileVName b : baseVNames) {
      Preconditions.checkNotNull(b.pattern, "pattern == null for base VName: %s", b.vname);
      Preconditions.checkNotNull(b.vname, "vname template == null for pattern: %s", b.pattern);
    }
    this.baseVNames = baseVNames;
    this.defaultCorpus = Suppliers.memoize(EnvironmentUtils::defaultCorpus);
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

  /** Returns a {@link FileVNames} parsed from the given JSON-encoded file. */
  public static FileVNames fromFile(String configFile) throws IOException {
    return fromFile(Paths.get(configFile));
  }

  /** Returns a {@link FileVNames} parsed from the given JSON-encoded file. */
  public static FileVNames fromFile(Path configFile) throws IOException {
    return new FileVNames(
        GSON.<List<BaseFileVName>>fromJson(
            Files.newBufferedReader(configFile, UTF_8), CONFIG_TYPE));
  }

  /** Returns a {@link FileVNames} parsed from the given JSON-encoded {@link String}. */
  public static FileVNames fromJson(String json) {
    return new FileVNames(GSON.<List<BaseFileVName>>fromJson(json, CONFIG_TYPE));
  }

  /**
   * Returns an equivalent {@link FileVNames} from the given sequence of {@link VNameRewriteRule}s.
   */
  public static FileVNames fromProto(VNameRewriteRules rules) {
    return fromProto(rules.getRuleList());
  }

  /**
   * Returns an equivalent {@link FileVNames} from the given sequence of {@link VNameRewriteRule}s.
   */
  public static FileVNames fromProto(Iterable<VNameRewriteRule> rules) {
    return new FileVNames(
        Streams.stream(rules)
            .map(
                r ->
                    new BaseFileVName(
                        Pattern.compile(r.getPattern()),
                        new VNameTemplate(
                            Strings.emptyToNull(r.getVName().getCorpus()),
                            Strings.emptyToNull(r.getVName().getRoot()),
                            Strings.emptyToNull(r.getVName().getPath()))))
            .collect(Collectors.toList()));
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
          return b.vname.fillInWith(matcher, defaultCorpus);
        }
      }
    }
    return VName.getDefaultInstance();
  }

  /** Returns an equivalent {@link VNameRewriteRules}. */
  public VNameRewriteRules toProto() {
    return VNameRewriteRules.newBuilder()
        .addAllRule(
            baseVNames.stream()
                .map(
                    b ->
                        VNameRewriteRule.newBuilder()
                            .setPattern(b.pattern.toString())
                            .setVName(
                                VName.newBuilder()
                                    .setCorpus(Strings.nullToEmpty(b.vname.corpus))
                                    .setRoot(Strings.nullToEmpty(b.vname.root))
                                    .setPath(Strings.nullToEmpty(b.vname.path))
                                    .build())
                            .build())
                .collect(Collectors.toList()))
        .build();
  }

  public String getDefaultCorpus() {
    return defaultCorpus.get();
  }

  /** Base {@link VName} to use for files matching {@code pattern}. */
  private static class BaseFileVName {
    public final Pattern pattern;
    public final VNameTemplate vname;

    public BaseFileVName(Pattern pattern, VNameTemplate vname) {
      this.pattern = pattern;
      this.vname = vname;
    }

    // GSON-required no-args constructor.
    private BaseFileVName() {
      this(null, null);
    }
  }

  /** Subset of a {@link VName} with built-in templating '@<group>@' markers. */
  private static class VNameTemplate {
    private final String corpus;
    private final String root;
    private final String path;

    public VNameTemplate(String corpus, String root, String path) {
      this.corpus = corpus;
      this.root = root;
      this.path = path;
    }

    // GSON-required no-args constructor.
    private VNameTemplate() {
      this(null, null, null);
    }

    /**
     * Returns a {@link VName} by filling in its corpus/root/path with regex groups in the given
     * {@link Matcher}.
     */
    public VName fillInWith(Matcher m, Supplier<String> defaultCorpus) {
      VName.Builder b = VName.newBuilder();
      if (corpus != null) {
        b.setCorpus(fillIn(corpus, m));
      } else {
        b.setCorpus(defaultCorpus.get());
      }
      if (root != null) {
        b.setRoot(fillIn(root, m));
      }
      if (path != null) {
        b.setPath(fillIn(path, m));
      }
      return b.build();
    }

    /**
     * Returns a {@link VName} by filling in its corpus/root/path with regex groups in the given
     * {@link Matcher}.
     */
    public VName fillInWith(Matcher m, String defaultCorpus) {
      return fillInWith(m, () -> defaultCorpus);
    }

    private static final Pattern replacerPattern = Pattern.compile("@(\\w+)@");

    private static String fillIn(String tmpl, Matcher m) {
      Matcher replacers = replacerPattern.matcher(tmpl);
      Deque<ReplacementMarker> matches = new ArrayDeque<>();
      while (replacers.find()) {
        matches.addFirst(
            new ReplacementMarker(replacers.start(), replacers.end(), replacers.group(1)));
      }
      StringBuilder builder = new StringBuilder(tmpl);
      while (!matches.isEmpty()) {
        ReplacementMarker res = matches.removeFirst();
        builder.replace(res.start, res.end, res.groupText(m));
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
    final String grp;

    ReplacementMarker(int start, int end, String grp) {
      this.start = start;
      this.end = end;
      this.grp = grp;
    }

    String groupText(Matcher m) {
      try {
        int idx = Integer.parseInt(grp);
        return m.group(idx);
      } catch (NumberFormatException unused) {
        return m.group(grp);
      }
    }
  }

  private static class PatternDeserializer implements JsonDeserializer<Pattern> {
    @Override
    public Pattern deserialize(JsonElement json, Type typeOfT, JsonDeserializationContext context) {
      return Pattern.compile(json.getAsJsonPrimitive().getAsString());
    }
  }
}
