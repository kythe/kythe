/*
 * Copyright 2016 Google Inc. All rights reserved.
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

import com.google.devtools.kythe.analyzers.base.EdgeKind;
import com.google.devtools.kythe.common.FormattingLogger;
import com.google.devtools.kythe.proto.Storage.VName;
import com.google.devtools.kythe.util.JsonUtil;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSyntaxException;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

/** Loads Kythe JSON-formatted metadata (with the .meta extension). */
public class KytheMetadataLoader implements MetadataLoader {
  private static final FormattingLogger logger =
      FormattingLogger.getLogger(KytheMetadataLoader.class);

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

  // TODO(zarko): Replace exceptions with some @AutoValue return type.
  private static Metadata.Rule parseRule(JsonObject json) throws JsonSyntaxException {
    JsonPrimitive ruleType = json.getAsJsonPrimitive(TYPE);
    if (ruleType == null) {
      throw new JsonSyntaxException("Rule missing type.");
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
          throw new JsonSyntaxException("Rule edge has zero length");
        }
        rule.reverseEdge = (edge.charAt(0) == '%');
        if (rule.reverseEdge) {
          edge = edge.substring(1);
        }
        if (edge.equals(GENERATES)) {
          rule.edgeOut = EdgeKind.GENERATES;
        } else {
          throw new JsonSyntaxException("Unknown edge type: " + edge);
        }
        return rule;
      default:
        throw new JsonSyntaxException("Unknown rule type: " + ruleType.getAsString());
    }
  }

  // TODO(zarko): Use AutoValue return here and only worry about IOException.
  @Override
  public Metadata parseFile(String fileName, byte[] data) {
    if (!fileName.endsWith(META_SUFFIX)) {
      return null;
    }
    try (ByteArrayInputStream stream = new ByteArrayInputStream(data);
        InputStreamReader reader = new InputStreamReader(stream, StandardCharsets.UTF_8)) {
      JsonElement root = PARSER.parse(reader);
      if (root == null || !root.isJsonObject()) {
        throw new JsonSyntaxException("Missing root.");
      }
      JsonObject rootObject = root.getAsJsonObject();
      JsonPrimitive rootType = rootObject.getAsJsonPrimitive(TYPE);
      if (rootType == null || !rootType.getAsString().equals(KYTHE_FORMAT_0)) {
        throw new JsonSyntaxException("Root missing type.");
      }
      JsonArray rules = rootObject.getAsJsonArray(META);
      if (rules == null) {
        throw new JsonSyntaxException("Root missing meta array.");
      }
      Metadata metadata = new Metadata();
      for (JsonElement rule : rules) {
        metadata.addRule(parseRule(rule.getAsJsonObject()));
      }
      return metadata;
    } catch (JsonSyntaxException ex) {
      logger.warning("JsonSyntaxException on " + fileName);
    } catch (IOException ex) {
      logger.warning("IOException on " + fileName);
    }
    return null;
  }
}
