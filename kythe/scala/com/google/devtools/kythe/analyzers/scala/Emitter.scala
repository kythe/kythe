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

import java.nio.file.Files

import com.google.devtools.kythe.proto.Schema.{EdgeKind, FactName, NodeKind}
import com.google.devtools.kythe.proto.Serving.File
import com.google.devtools.kythe.proto.Span
import com.google.devtools.kythe.util.Normalizer
import com.google.devtools.kythe.util.schema.Schema
import com.google.protobuf.ByteString

import scala.io.Source
import scala.reflect.internal.util.RangePosition
import scala.reflect.io.AbstractFile

/**
 * Interface/helper routines for emitting nodes and edges.
 */
trait Emitter extends AddSigs {
  val global: scala.tools.nsc.Global
  import global._

  /**
   * Emitters can be written to write data out as json objects
   * or as protocol buffer objects.
   * NODE is used as the type to represent VNames for the emitter
   * we use.
   * Essentially it is a tuple of (signature, language, corpus) where
   * signature is used to uniquely identify every symbol in language/corpus.
   * @see http://www.kythe.io/docs/kythe-storage.html#_a_id_termnode_a_nodes_as_vectors
   * for more details on VNames.
   */
  type NODE
  val CORPUS = sys.env.get("KYTHE_CORPUS").getOrElse("")

  // init gets called once for each compilation unit.
  def init(filename: AbstractFile, outDirName: String): Unit
  def currentFile(): AbstractFile
  def flush(): Unit

  def emitEdge(source: NODE, edgeKind: EdgeKind, target: NODE): Unit
  def emitFact(vName: NODE, factName: FactName, factValue: String): Unit
  def emitParamEdge(source: NODE, target: NODE, position: Int): Unit

  def nodeVName(
      signature: String,
      path: String,
      language: String = "scala"
  ): NODE
  def fileVName(path: String): NODE
  def anchorVName(signature: String, path: String): NODE

  case class AnchorDetails(
      filePath: String,
      start: Int,
      end: Int,
      lineStart: Int,
      lineEnd: Int,
      colStart: Int,
      colEnd: Int
  )

  // Emit a facts for a file node and for a name node for the file
  // The name for a file is just the basename, no directory.
  def emitFileFacts(path: String, content: Array[Char]): NODE = {
    // file node
    val fName = fileVName(path)
    emitFact(fName, FactName.NODE_KIND, Schema.nodeKindString(NodeKind.FILE))
    emitFact(fName, FactName.TEXT, new String(content))
    // name node and edge
    val nameVName = nodeVName(path.split(java.io.File.separator).last, "")
    emitFact(
      nameVName,
      FactName.NODE_KIND,
      Schema.nodeKindString(NodeKind.NAME)
    )
    emitEdge(fName, EdgeKind.NAMED, nameVName)
    fName
  }

  /**
   * Emitting an anchor emits:
   *    the start/end location facts
   *    the childof edge to the file
   *
   * @param anchorLocation Where in the source code to put the anchor
   * @param anchorPointsTo Which node the anchor should point to
   * @param refKind which type of anchor
   */
  def emitAnchor(
      anchorLocation: AnchorDetails,
      anchorPointsTo: NODE,
      refKind: EdgeKind
  ): NODE = {
    val anchor = anchorVName(anchorLocation.toString, anchorLocation.filePath)
    emitFact(anchor, FactName.NODE_KIND, Schema.nodeKindString(NodeKind.ANCHOR))
    emitFact(anchor, FactName.LOC_START, anchorLocation.start.toString)
    emitFact(anchor, FactName.LOC_END, anchorLocation.end.toString)
    emitEdge(anchor, EdgeKind.CHILD_OF, fileVName(anchorLocation.filePath))
    emitEdge(anchor, refKind, anchorPointsTo)
    anchor
  }

  /**
   * Big Hack!
   * We only want to emit anchors (links) for things that are explicit in the
   * original scala source code.  We maintain structure (like callgraphs) for code
   * the compiler generates for us, but we should not be able to click on links
   * there (yet).  It messes up the current UIs that we have.  Ideally we let the UI know
   * which anchors are originally there.  The current plugin architecture doesn't give
   * a good way to figure this out for references (Apply).
   * The hack we employ is to check the original code for the symbol name.
   * This doesn't weed out all the extra links, but it does remove most of them.
   */
  def isSymbolInRange(tree: Tree, symbol: Symbol) =
    tree.pos.lineContent.contains(symbol.nameString)

  def symbolForTree(tree: Tree): Option[Symbol] = {
    // tree.symbol will be null on 'val global: Kythe.this.global.type = Kythe.this.global'
    if (tree.pos != NoPosition && tree.symbol != null) {
      Some(
        if (tree.symbol.accessedOrSelf == NoSymbol) tree.symbol
        else tree.symbol.accessedOrSelf
      )
    } else None
  }

  def emitAnchor(tree: Tree, kind: EdgeKind): Option[NODE] = {
    symbolForTree(tree).flatMap(symbol => emitAnchor(tree, symbol, kind))
  }

  /**
   * @return VName of emitted anchor or None if tree had no position.
   */
  def emitAnchor(tree: Tree, symbol: Symbol, kind: EdgeKind): Option[NODE] = {
    // If we can't get the true end from the symbol so we calculate it.
    // For define anchor and ref/calls, we want the whole range.
    // For defs, we want the range of the symbol
    // We have this problem even if we add -yrange to the analyzer.
    val (start, end) = tree.pos match {
      case _: RangePosition if (tree.isDef && kind != EdgeKind.DEFINES) =>
        (tree.pos.point, tree.pos.point + symbol.nameString.length)

      case _: RangePosition => (tree.pos.start, tree.pos.end)

      // Deal with people not passing -Yrangepos, even though we need it for a good index.
      case _ => (tree.pos.point, tree.pos.point + symbol.nameString.length)
    }

    val normalizer = new Normalizer(
      File
        .newBuilder()
        .setEncoding("UTF-8")
        .setText(
          ByteString.copyFrom(Files.readAllBytes(currentFile().file.toPath))
        )
        .build()
    )
    val points: Span = normalizer.expandPoints(start, end)

    val details = AnchorDetails(
      tree.pos.source.file.path,
      start,
      end,
      points.getStart.getLineNumber,
      points.getEnd.getLineNumber,
      points.getStart.getColumnOffset,
      points.getEnd.getColumnOffset
    )
    val node = nodeVName(generateSignature(symbol), tree.pos.source.file.path)
    // TODO(rbraunstein): revisit to make sure we do this everywhere needed.
    // if (tree.symbol.isJavaDefined)nodeVName(javaSymbolicName(symbol), NO_PATH, "java") else
    if (isSymbolInRange(tree, symbol))
      Some(emitAnchor(details, node, kind))
    else None
  }

  def emitNodeKind(vname: NODE, kind: NodeKind): Unit =
    emitFact(vname, FactName.NODE_KIND, Schema.nodeKindString(kind))

}
