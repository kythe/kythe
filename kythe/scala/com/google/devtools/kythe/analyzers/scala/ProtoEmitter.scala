/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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
package com.google.devtools.kythe.analyzers.scala

import java.io.{File, FileOutputStream, OutputStream}

import com.google.protobuf.ByteString
import com.google.devtools.kythe.proto.Storage.Entry
import com.google.devtools.kythe.proto.Storage.VName
import java.nio.charset.Charset
import java.util.Base64

import com.google.devtools.kythe.proto.Schema.{EdgeKind, FactName}
import com.google.devtools.kythe.util.schema.Schema

import scala.reflect.io.AbstractFile
import scala.tools.nsc
import nsc.Global

/**
 * An Emitter that emits kythe.proto.Entry to stdout
 */
class ProtoEmitter[G <: Global](val global: G) extends Emitter {
  import global._
  type NODE = VName

  var filename: AbstractFile = _

  val PREFIX_TO_TRUNCATE = sys.env.get("KYTHE_REMOVE_PREFIX").getOrElse("")
  def removePrefix(path: String): String = {
    if (path.startsWith(PREFIX_TO_TRUNCATE)) {
      path.substring(PREFIX_TO_TRUNCATE.length)
    } else {
      path
    }
  }

  def emitEdge(source: NODE, kind: EdgeKind, target: NODE): Unit = {
    Entry
      .newBuilder()
      .setEdgeKind(Schema.edgeKindString(kind))
      .setSource(source)
      .setTarget(target)
      .setFactName("/") // silly, but needed
      .build()
      .writeDelimitedTo(out)
  }

  def emitParamEdge(source: NODE, target: NODE, position: Int): Unit = {
    Entry
      .newBuilder()
      .setEdgeKind(s"${Schema.edgeKindString(EdgeKind.PARAM)}.${position}")
      .setSource(source)
      .setTarget(target)
      .setFactName("/") // silly, but needed
      .build()
      .writeDelimitedTo(out)
  }

  def emitFact(vName: NODE, factName: FactName, factValue: String): Unit = {
    val bytes = ByteString.copyFromUtf8(factValue)
    // do I need to encode bytes?
    Entry
      .newBuilder()
      .setSource(vName)
      .setFactName(Schema.factNameString(factName))
      .setFactValue(bytes)
      .build()
      .writeDelimitedTo(out)
  }

  def nodeVName(
      signature: String,
      path: String,
      language: String = "scala"
  ): NODE = {
    VName
      .newBuilder()
      .setCorpus(CORPUS)
      .setPath(removePrefix(path))
      //.setRoot("/tmp")
      .setSignature(signature)
      .setLanguage(language)
      .build()
  }

  /**
   * File vnames are special.  The signature and language should be cleared.
   */
  def fileVName(path: String): NODE = {
    VName
      .newBuilder()
      .setCorpus(CORPUS)
      .setPath(removePrefix(path))
      .setSignature("")
      .setLanguage("")
      .build()
  }

  def anchorVName(signature: String, path: String): NODE = {
    VName
      .newBuilder()
      .setCorpus(CORPUS)
      .setSignature(signature)
      .setPath(removePrefix(path))
      .build()
  }

  var out: OutputStream = System.out
  val NL = "\n".getBytes(Charset.forName("UTF-8"))

  override def currentFile(): AbstractFile = {
    filename
  }

  override def init(filename: AbstractFile, outDirName: String): Unit = {
    this.filename = filename
    out =
      if (outDirName == "-") System.out
      else {
        val outDir = new File(outDirName)
        outDir.mkdirs()
        new FileOutputStream(new File(outDir, filename.file.getName))
      }
  }
  override def flush(): Unit = {
    out.flush()
  }

  // Values should be base64 encoded
  def encodeValue(value: String) =
    Base64.getEncoder.encodeToString(value.getBytes(Charset.forName("UTF-8")))
}

object ProtoEmitter {
  def apply[G <: Global](global: G): ProtoEmitter[G] = new ProtoEmitter(global)
}
