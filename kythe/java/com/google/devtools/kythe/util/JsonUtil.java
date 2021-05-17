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

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.io.BaseEncoding;
import com.google.devtools.kythe.proto.Buildinfo.BuildDetails;
import com.google.devtools.kythe.proto.Java.JavaDetails;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonParseException;
import com.google.gson.JsonParser;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.google.protobuf.ByteString;
import com.google.protobuf.GeneratedMessageV3;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.LazyStringArrayList;
import com.google.protobuf.LazyStringList;
import com.google.protobuf.ProtocolMessageEnum;
import com.google.protobuf.util.JsonFormat;
import java.lang.reflect.Type;
import java.util.ArrayList;

/** Utility class for working with JSON/{@link Gson}. */
public class JsonUtil {
  private JsonUtil() {}

  /** Types that may be present in JSON pb files */
  public static final JsonFormat.TypeRegistry.Builder JSON_TYPE_REGISTRY =
      JsonFormat.TypeRegistry.newBuilder()
          .add(JavaDetails.getDescriptor())
          .add(BuildDetails.getDescriptor());

  /** The registry currently in use. */
  private static JsonFormat.TypeRegistry registry;

  /** Use the given {@link JsonFormat.TypeRegistry.Builder} when parsing proto3 Any messages. */
  public static void usingTypeRegistry(JsonFormat.TypeRegistry.Builder registryBuilder) {
    usingTypeRegistry(registryBuilder.build());
  }

  /** Use the given {@link JsonFormat.TypeRegistry} when parsing proto3 Any messages. */
  public static void usingTypeRegistry(JsonFormat.TypeRegistry registry) {
    // usingTypeRegistry() throws IllegalArgumentException if called a second time.
    // However we may need to call it multiple times, e.g., if running main() multiple
    // times in a nailgun server.
    // We support this, as long as it's called with the same registry every time.
    if (registry.equals(JsonUtil.registry)) {
      return;
    }
    GeneratedMessageV3TypeAdapter.PARSER =
        GeneratedMessageV3TypeAdapter.PARSER.usingTypeRegistry(registry);
    GeneratedMessageV3TypeAdapter.PRINTER =
        GeneratedMessageV3TypeAdapter.PRINTER.usingTypeRegistry(registry);
    JsonUtil.registry = registry;
  }

  /**
   * Registers type adapters for Java protobuf types (including ByteStrings and byte[]) that matches
   * Go's JSON encoding/decoding.
   */
  public static GsonBuilder registerProtoTypes(GsonBuilder builder) {
    return builder
        .registerTypeHierarchyAdapter(GeneratedMessageV3.class, new GeneratedMessageV3TypeAdapter())
        .registerTypeHierarchyAdapter(ProtocolMessageEnum.class, new ProtoEnumTypeAdapter())
        .registerTypeHierarchyAdapter(ByteString.class, new ByteStringTypeAdapter())
        .registerTypeAdapter(byte[].class, new ByteArrayTypeAdapter())
        .registerTypeHierarchyAdapter(LazyStringList.class, new LazyStringListTypeAdapter());
  }

  private static class GeneratedMessageV3TypeAdapter
      implements JsonSerializer<GeneratedMessageV3>, JsonDeserializer<GeneratedMessageV3> {
    private static JsonFormat.Parser PARSER = JsonFormat.parser();
    private static JsonFormat.Printer PRINTER =
        JsonFormat.printer().preservingProtoFieldNames().omittingInsignificantWhitespace();

    @Override
    public JsonElement serialize(GeneratedMessageV3 msg, Type t, JsonSerializationContext ctx) {
      try {
        return JsonParser.parseString(PRINTER.print(msg));
      } catch (InvalidProtocolBufferException e) {
        throw new RuntimeException(e);
      }
    }

    @Override
    public GeneratedMessageV3 deserialize(
        JsonElement json, Type typeOfT, JsonDeserializationContext context)
        throws JsonParseException {
      try {
        Class<? extends GeneratedMessageV3> protoClass =
            (Class<? extends GeneratedMessageV3>) typeOfT;
        GeneratedMessageV3.Builder<?> protoBuilder =
            (GeneratedMessageV3.Builder<?>) protoClass.getMethod("newBuilder").invoke(null);
        String msg = json instanceof JsonPrimitive ? json.getAsString() : json.toString();
        PARSER.merge(msg, protoBuilder);
        return (GeneratedMessageV3) protoBuilder.build();
      } catch (ReflectiveOperationException e) {
        throw new JsonParseException(
            "failed to retrieve Message.Builder while parsing proto3 message", e);
      } catch (InvalidProtocolBufferException e) {
        throw new JsonParseException(e);
      }
    }
  }

  private static class ByteStringTypeAdapter
      implements JsonSerializer<ByteString>, JsonDeserializer<ByteString> {
    @Override
    public JsonElement serialize(ByteString str, Type t, JsonSerializationContext ctx) {
      return ctx.serialize(str.toByteArray());
    }

    @Override
    public ByteString deserialize(
        JsonElement json, Type typeOfT, JsonDeserializationContext context) {
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
    public byte[] deserialize(JsonElement json, Type typeOfT, JsonDeserializationContext context) {
      return ENCODING.decode((String) context.deserialize(json, String.class));
    }
  }

  private static class LazyStringListTypeAdapter
      implements JsonSerializer<LazyStringList>, JsonDeserializer<LazyStringList> {
    @Override
    public JsonElement serialize(LazyStringList lsl, Type t, JsonSerializationContext ctx) {
      ArrayList<String> elements = new ArrayList<>(lsl.size());
      for (byte[] element : lsl.asByteArrayList()) {
        elements.add(new String(element, UTF_8));
      }
      return ctx.serialize(elements);
    }

    @Override
    public LazyStringList deserialize(JsonElement json, Type t, JsonDeserializationContext ctx) {
      if (json.isJsonNull()) {
        return null;
      }
      LazyStringList lsl = new LazyStringArrayList();
      for (JsonElement element : json.getAsJsonArray()) {
        lsl.add((String) ctx.deserialize(element, String.class));
      }
      return lsl;
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
    public ProtocolMessageEnum deserialize(
        JsonElement json, Type t, JsonDeserializationContext ctx) {
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
