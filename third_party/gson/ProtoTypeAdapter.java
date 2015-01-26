/*
 * Copyright (C) 2010 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.gson.protobuf;

import com.google.gson.JsonArray;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParseException;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.EnumValueDescriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;
import com.google.protobuf.GeneratedMessage;
import com.google.protobuf.ProtocolMessageEnum;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Gson type adapter for protocol buffers
 *
 * @author Inderjeet Singh
 */
public class ProtoTypeAdapter implements JsonSerializer<GeneratedMessage>,
    JsonDeserializer<GeneratedMessage> {

  @Override
  public JsonElement serialize(GeneratedMessage src, Type typeOfSrc,
      JsonSerializationContext context) {
    JsonObject ret = new JsonObject();
    final Map<FieldDescriptor, Object> fields = src.getAllFields();

    for (Map.Entry<FieldDescriptor, Object> fieldPair : fields.entrySet()) {
      final FieldDescriptor desc = fieldPair.getKey();
      if (fieldPair.getValue() instanceof EnumValueDescriptor) {
        EnumValueDescriptor valDesc = (EnumValueDescriptor) fieldPair.getValue();
        ret.add(desc.getName(), context.serialize(valDesc.getNumber()));
      } else {
        ret.add(desc.getName(), context.serialize(fieldPair.getValue()));
      }
    }
    return ret;
  }

  @SuppressWarnings("unchecked")
  @Override
  public GeneratedMessage deserialize(JsonElement json, Type typeOfT,
      JsonDeserializationContext context) throws JsonParseException {
    try {
      JsonObject jsonObject = json.getAsJsonObject();
      Class<? extends GeneratedMessage> protoClass =
        (Class<? extends GeneratedMessage>) typeOfT; 
      try {
        // Invoke the ProtoClass.newBuilder() method
        Object protoBuilder = getCachedMethod(protoClass, "newBuilder")
          .invoke(null);
        Class<?> builderClass = protoBuilder.getClass();

        Descriptor protoDescriptor = (Descriptor) getCachedMethod(
            protoClass, "getDescriptor").invoke(null);
        // Call setters on all of the available fields
        for (FieldDescriptor fieldDescriptor : protoDescriptor.getFields()) {
          String name = fieldDescriptor.getName();
          if (jsonObject.has(name)) {
            JsonElement jsonElement = jsonObject.get(name);
            String fieldName = getJavaFieldName(name);
            Field field = protoClass.getDeclaredField(fieldName);
            Type fieldType = field.getGenericType();
            Object fieldValue = context.deserialize(jsonElement, fieldType);
            if (fieldValue instanceof ProtocolMessageEnum) {
              fieldValue = ((ProtocolMessageEnum) fieldValue).getValueDescriptor();
            }
            Method method = getCachedMethod(
              builderClass, "setField", FieldDescriptor.class, Object.class);
            method.invoke(protoBuilder, fieldDescriptor, fieldValue);
          }
        }
        
        // Invoke the build method to return the final proto
        return (GeneratedMessage) getCachedMethod(builderClass, "build")
            .invoke(protoBuilder);
      } catch (SecurityException e) {
        throw new JsonParseException(e);
      } catch (NoSuchMethodException e) {
        throw new JsonParseException(e);
      } catch (IllegalArgumentException e) {
        throw new JsonParseException(e);
      } catch (IllegalAccessException e) {
        throw new JsonParseException(e);
      } catch (InvocationTargetException e) {
        throw new JsonParseException(e);
      }
    } catch (Exception e) {
      throw new JsonParseException("Error while parsing proto: ", e);
    }
  }

  private static String getJavaFieldName(String name) {
    String[] parts = name.split("_");
    StringBuilder fieldName = new StringBuilder(parts[0]);
    for (int i = 1; i < parts.length; i++) {
      fieldName.append(Character.toUpperCase(parts[i].charAt(0)));
      fieldName.append(parts[i].substring(1));
    }
    fieldName.append('_');
    return fieldName.toString();
  }

  private static Method getCachedMethod(Class<?> clazz, String methodName,
      Class<?>... methodParamTypes) throws NoSuchMethodException {
    Map<Class<?>, Method> mapOfMethods = mapOfMapOfMethods.get(methodName);
    if (mapOfMethods == null) {
      mapOfMethods = new HashMap<Class<?>, Method>();
      mapOfMapOfMethods.put(methodName, mapOfMethods);
    }
    Method method = mapOfMethods.get(clazz);
    if (method == null) {
      method = clazz.getMethod(methodName, methodParamTypes);
      mapOfMethods.put(clazz, method);
    }
    return method;
  }

  private static Map<String, Map<Class<?>, Method>> mapOfMapOfMethods =
    new HashMap<String, Map<Class<?>, Method>>();
}
