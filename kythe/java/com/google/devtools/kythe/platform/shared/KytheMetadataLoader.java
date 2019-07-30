/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

package com.google.devtools.kythe.platform.shared;

import com.google.auto.value.AutoValue;
import com.google.common.flogger.FluentLogger;
import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonPrimitive;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import org.checkerframework.checker.nullness.qual.Nullable;

/** Loads Kythe JSON-formatted metadata (with the .meta extension). */
public class KytheMetadataLoader implements MetadataLoader {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  /** This extension signifies a Kythe JSON metadata file. */
  private static final String META_SUFFIX = ".meta";

  private static final String NOP = "nop";
  private static final String TYPE = "type";
  private static final String VNAME = "vname";
  private static final String EDGE = "edge";
  private static final String ANCHOR_DEFINES = "anchor_defines";
  private static final String BEGIN = "begin";
  private static final String END = "end";
  private static final String GENERATES = "/kythe/edge/generates";
  private static final String KYTHE_FORMAT_0 = "kythe0";
  private static final String META = "meta";

  private static final Gson GSON = JsonUtil.registerProtoTypes(new GsonBuilder()).create();
  private static final JsonParser PARSER = new JsonParser();

  @AutoValue
  abstract static class RuleXorError {
    static RuleXorError create(Metadata.Rule rule) {
      return new AutoValue_KytheMetadataLoader_RuleXorError(rule, null);
    }

    static RuleXorError create(String error) {
      return new AutoValue_KytheMetadataLoader_RuleXorError(null, error);
    }

    abstract Metadata.@Nullable Rule rule();

    @Nullable
    abstract String error();
  }

  @Nullable
  private static RuleXorError parseRule(JsonObject json) {
    JsonPrimitive ruleType = json.getAsJsonPrimitive(TYPE);
    if (ruleType == null) {
      return RuleXorError.create("Rule missing type.");
    }
    switch (ruleType.getAsString()) {
      case NOP:
        return null;
      case ANCHOR_DEFINES:
        Metadata.Rule rule = new Metadata.Rule();
        rule.vname = GSON.fromJson(json.getAsJsonObject(VNAME), VName.class);
        rule.begin = json.getAsJsonPrimitive(BEGIN).getAsInt();
        rule.end = json.getAsJsonPrimitive(END).getAsInt();
        String edge = json.getAsJsonPrimitive(EDGE).getAsString();
        if (edge.length() == 0) {
          return RuleXorError.create("Rule edge has zero length");
        }
        rule.reverseEdge = (edge.charAt(0) == '%');
        if (rule.reverseEdge) {
          edge = edge.substring(1);
        }
        if (edge.equals(GENERATES)) {
          rule.edgeOut = EdgeKind.GENERATES;
        } else {
          return RuleXorError.create("Unknown edge type: " + edge);
        }
        return RuleXorError.create(rule);
      default:
        return RuleXorError.create("Unknown rule type: " + ruleType.getAsString());
    }
  }

  @Override
  public Metadata parseFile(String fileName, byte[] data) {
    if (!fileName.endsWith(META_SUFFIX)) {
      return null;
    }
    JsonElement root = null;
    try (ByteArrayInputStream stream = new ByteArrayInputStream(data);
        InputStreamReader reader = new InputStreamReader(stream, StandardCharsets.UTF_8)) {
      root = PARSER.parse(reader);
    } catch (IOException ex) {
      emitWarning(ex.getMessage(), fileName);
      return null;
    }
    if (root == null || !root.isJsonObject()) {
      emitWarning("Missing root.", fileName);
      return null;
    }
    JsonObject rootObject = root.getAsJsonObject();
    JsonPrimitive rootType = rootObject.getAsJsonPrimitive(TYPE);
    if (rootType == null || !rootType.getAsString().equals(KYTHE_FORMAT_0)) {
      emitWarning("Root missing type.", fileName);
      return null;
    }
    JsonArray rules = rootObject.getAsJsonArray(META);
    if (rules == null) {
      emitWarning("Root missing meta array.", fileName);
      return null;
    }
    Metadata metadata = new Metadata();
    for (JsonElement rule : rules) {
      RuleXorError ruleXorError = parseRule(rule.getAsJsonObject());
      if (ruleXorError != null) { // skip nulls
        Metadata.Rule metadataRule = ruleXorError.rule();
        if (metadataRule != null) {
          metadata.addRule(metadataRule);
        } else {
          emitWarning(ruleXorError.error(), fileName);
        }
      }
    }
    return metadata;
  }

  private static void emitWarning(String warning, String fileName) {
    logger.atWarning().log("Error in parsing rule: %s\nfrom file: %s", warning, fileName);
  }
}
