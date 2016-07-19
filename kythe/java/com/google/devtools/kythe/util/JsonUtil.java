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

import com.google.common.io.BaseEncoding;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonParseException;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.google.gson.protobuf.ProtoTypeAdapter;
import com.google.protobuf.ByteString;
import com.google.protobuf.GeneratedMessage;
import com.google.protobuf.ProtocolMessageEnum;
import java.lang.reflect.Type;

/** Utility class for working with JSON/{@link Gson}. */
public class JsonUtil {

  /**
   * Registers type adapters for Java protobuf types (including ByteStrings and byte[]) that matches
   * Go's JSON encoding/decoding.
   */
  public static GsonBuilder registerProtoTypes(GsonBuilder builder) {
    return builder
        .registerTypeHierarchyAdapter(ProtocolMessageEnum.class, new ProtoEnumTypeAdapter())
        .registerTypeHierarchyAdapter(GeneratedMessage.class, new ProtoTypeAdapter())
        .registerTypeHierarchyAdapter(ByteString.class, new ByteStringTypeAdapter())
        .registerTypeAdapter(byte[].class, new ByteArrayTypeAdapter());
  }

  private static class ByteStringTypeAdapter
      implements JsonSerializer<ByteString>, JsonDeserializer<ByteString> {
    @Override
    public JsonElement serialize(ByteString str, Type t, JsonSerializationContext ctx) {
      return ctx.serialize(str.toByteArray());
    }

    @Override
    public ByteString deserialize(
        JsonElement json, Type typeOfT, JsonDeserializationContext context)
        throws JsonParseException {
      return ByteString.copyFrom((byte[]) context.deserialize(json, byte[].class));
    }
  }

  private static class ByteArrayTypeAdapter
      implements JsonSerializer<byte[]>, JsonDeserializer<byte[]> {
    private static final BaseEncoding ENCODING = BaseEncoding.base64();

    @Override
    public JsonElement serialize(byte[] arry, Type t, JsonSerializationContext ctx) {
      return new JsonPrimitive(ENCODING.encode(arry));
    }

    @Override
    public byte[] deserialize(JsonElement json, Type typeOfT, JsonDeserializationContext context)
        throws JsonParseException {
      return ENCODING.decode((String) context.deserialize(json, String.class));
    }
  }

  // Type adapter for bare protobuf enum values.
  private static class ProtoEnumTypeAdapter
      implements JsonSerializer<ProtocolMessageEnum>, JsonDeserializer<ProtocolMessageEnum> {
    @Override
    public JsonElement serialize(ProtocolMessageEnum e, Type t, JsonSerializationContext ctx) {
      return new JsonPrimitive(e.getNumber());
    }

    @Override
    @SuppressWarnings("unchecked")
    public ProtocolMessageEnum deserialize(JsonElement json, Type t, JsonDeserializationContext ctx)
        throws JsonParseException {
      int num = json.getAsJsonPrimitive().getAsInt();
      Class<? extends ProtocolMessageEnum> enumClass = (Class<? extends ProtocolMessageEnum>) t;
      try {
        return (ProtocolMessageEnum) enumClass.getMethod("valueOf", int.class).invoke(null, num);
      } catch (Exception e) {
        throw new JsonParseException(e);
      }
    }
  }
}
