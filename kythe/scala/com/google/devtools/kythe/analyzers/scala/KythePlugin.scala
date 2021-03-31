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

import com.google.devtools.kythe.proto.Schema.{
  EdgeKind,
  FactName,
  NodeKind,
  Subkind
}
import com.google.devtools.kythe.util.schema.Schema

import scala.tools.nsc
import scala.tools.nsc.{Global, Phase}
import scala.tools.nsc.plugins.{Plugin, PluginComponent}

/**
 * Plugin for the scalac compiler that generates kythe data
 */
class Kythe(val global: Global) extends Plugin with AddSigs {

  import global._

  val name = "kythe"
  val description = "Emits kythe data in json format"
  val components = List[PluginComponent](NamedComponent)
  // If the user sets an output directory then write json files to that directory.
  // Otherwise write protos to stdout
  var OUT_DIR = sys.env.get("KYTHE_OUTPUT_DIRECTORY").getOrElse("index")

  /**
   * Main interface for defining a compiler plugin component.
   *
   * @see http://www.scala-lang.org/old/node/140
   */
  private object NamedComponent extends PluginComponent {
    val global: Kythe.this.global.type = Kythe.this.global
    val runsAfter = List[String]("refchecks");
    val phaseName = Kythe.this.name
    // TODO(rbraunstein): find the right way to write this in scala
    // if (OUT_DIR == "-") ProtoEmitter[global.type](global) else JsonEmitter[global.type](global)
    val emitter = ProtoEmitter[global.type](global)

    def newPhase(_prev: Phase) = new KythePhase(_prev)

    class KythePhase(prev: Phase) extends StdPhase(prev) {

      override def name = Kythe.this.name

      def apply(unit: CompilationUnit): Unit = processUnit(unit)

      // TODO(rbraunstein): do we ever get called for java units (class files?)
      def processUnit(unit: CompilationUnit) =
        if (!unit.isJava) processScalaUnit(unit)

      def processScalaUnit(unit: CompilationUnit): Unit = {
        emitter.init(unit.source.file, OUT_DIR)
        val fileVName =
          emitter.emitFileFacts(unit.source.file.path, unit.source.content)
        DefinitionTraverser(fileVName).apply(unit.body)
        emitter.flush
      }

      /**
       * Determine the kythe node kind for the tree
       */
      private def kytheType(tree: Tree): NodeKind = tree match {
        case _: ModuleDef => NodeKind.FUNCTION
        case _: ClassDef  => NodeKind.RECORD
        case _: ValDef    => NodeKind.VARIABLE
        case _: DefDef =>
          if (tree.symbol.accessedOrSelf.isMethod) NodeKind.FUNCTION
          else NodeKind.VARIABLE
        case _: PackageDef =>
          NodeKind.PACKAGE // TODO(rbraunstein): fix packages
      }

      /**
       * Determine if a Def exists in source code or if it is generated.
       * Most generated methods have the Synthetic flag on the symbol.
       * The one exception I found so far is <init>
       * I've special cased for the name "<init>" until we find a better solution.
       * TODO(rbraunstein): We may be able to get rid of this now.
       */
      private def isNotGeneratedDef(tree: Tree) =
        !tree.symbol.isSynthetic && tree.symbol.nameString != "<init>"

      /**
       * Emit the right subkinds and facts for variables nodes.
       * i.e. is it a 'param' or a 'field.
       * And write the ChildOf or Param edge back to its owner.
       */
      private def emitVariableExtras(tree: ValOrDefDef): Unit = {
        val vName = emitter.nodeVName(
          generateSignature(tree.symbol),
          tree.pos.source.file.path
        )
        val symbol = tree.symbol.accessedOrSelf
        if (symbol.hasFlag(scala.reflect.internal.Flags.LOCAL)) {
          emitter.emitFact(
            vName,
            FactName.SUBKIND,
            Schema.subkindString(Subkind.FIELD)
          )
          emitter.emitEdge(
            vName,
            EdgeKind.CHILD_OF,
            emitter.nodeVName(
              generateSignature(symbol.owner),
              tree.pos.source.file.path
            )
          )
        } else if (symbol.isParameter) {
          emitter.emitFact(
            vName,
            FactName.SUBKIND,
            Schema.subkindString(Subkind.LOCAL_PARAMETER)
          )
          emitter.emitParamEdge(
            emitter.nodeVName(
              generateSignature(symbol.owner),
              tree.pos.source.file.path
            ),
            vName,
            tree.symbol.paramPos
          )
        } else
          emitter.emitFact(
            vName,
            FactName.SUBKIND,
            Schema.subkindString(Subkind.LOCAL)
          )
      }

      /**
       * For the current variable, method, class, param, etc. definition, emit an edge to its type.
       * If the type is a compound type, we only point to the parent type
       * In the type visitor, we create the compound type
       * For compound types, we should emit a 'typed' edge from the given symbol defintion to a
       * 'tapp' node.
       * We should also emit a name node for tapp, which might already exist.
       *
       * Given the example:
       *     var foo: Map[String, List[String]]
       *
       * We want to write that the type of foo is the Node
       *    "Map[String,List[String]"
       *
       *    That Node has kind Tapp
       *              has [named] edge to name: "java.util.Map[java.lang.String,java.util.List[java.lang.String]]
       *              has param edges to:
       *                  Map [abs]
       *                  String [tbuiltin]
       *                  List [tapp]
       *
       *     For method definitions, its type is a tuple of:
       *         |function|
       *         return type
       *         parameter types.
       *
       *  Given the example:
       *      var foo: Integer
       *
       *  The type is simply "builtin#Integer"
       *
       *  For the example:
       *       var foo: MyClass
       *  The type points to the record for MyClass.
       *
       *  NOTE that we will emit type nodes multiple times.
       *  For the example:
       *      val s: Map[String, List[String]]
       *      val t: Map[String, List[String]]
       *    We will emit the tapp node and its subgraph for "Map[String, List[String]]" twice.
       */
      private def emitTypeEdgesForDefinition(
          tree: Tree,
          sourceNode: emitter.NODE
      ): Unit = {

        tree match {
          // method, macro
          // TODO(rbraunstein): implement method types, this is just anchors
          case defdef: DefDef =>
            emitAnchorForType(
              defdef,
              makeNodeForType(defdef.tpt.tpe, tree.pos.source.file.path)
            )

          // vals, vars, params
          case vd: ValDef =>
            emitTypeEdgesForVariable(vd, sourceNode, tree.pos.source.file.path)

          // classes, traits
          case _: ClassDef =>
            emitTypeEdgesForClassDef(
              tree.symbol,
              sourceNode,
              tree.pos.source.file.path
            )
          // object
          // TODO(rbraunstein): verify that objects have the correct type
          //  especially when extending a class.
          case _: ModuleDef => ()
          // packages have  no types
          case _: PackageDef => ()
        }
      }

      /*
       * The location of the type is no longer stored in the current tree.
       * We need to look at the original tree.
       */
      private def emitAnchorForType(
          tree: ValOrDefDef,
          typeNode: emitter.NODE
      ): Unit = {
        tree.tpt match {
          case tt: TypeTree =>
            if (tt.original != null)
              emitter.emitAnchor(tt.original, EdgeKind.REF)
          case _ => ()
        }
      }

      /**
       * Given a Type, make a VName for it.
       * It should be either:
       *     - a tapp node
       *     - a tbuiltin
       *     - the vname for a class node
       *     - an absvar node)
       *
       * NOTE: If its a tapp node, we make the new node and construct the subgraph too.
       * If not tapp node, just return the vname for the type.  Assume someone else created it.
       */
      private def makeNodeForType(
          symbolType: Type,
          path: String
      ): emitter.NODE = {
        // Tapp nodes need to be created in case they don't exist
        if (symbolType.typeArguments.nonEmpty) {
          val varTypeNode =
            emitter.nodeVName(makeSigForTappNode(symbolType), path)
          emitter.emitNodeKind(varTypeNode, NodeKind.TAPP)
          emitTappSubGraph(varTypeNode, symbolType, path)
          varTypeNode
        } else {
          val varTypeNode =
            emitter.nodeVName(generateSignature(symbolType.typeSymbol), path)
          // primitive and class nodes should already exist, just return the VName of the type.
          emitter.emitNodeKind(
            varTypeNode,
            if (symbolType.typeSymbol.isPrimitiveValueClass) NodeKind.TBUILTIN
            else if (symbolType.typeSymbol.isTypeParameter) NodeKind.ABSVAR
            else NodeKind.RECORD
          )
          varTypeNode
        }
      }

      /**
       * Recursively emit a tapp type.
       * For Map[String, List[String]]
       *   The initial tapp node has three params.
       *   The first is the abs type (Map)
       *   The next params are the top level types: String (primitive), List[String]: (tapp)
       *
       *   We need to:
       *     create vnames for each param,
       *     emit edges from this tapp to each param
       *     recursively visit any tapp node (creating new node as well)
       */
      def emitTappSubGraph(
          typeNode: emitter.NODE,
          thisType: Type,
          path: String
      ): Unit = {
        // monomorphic
        if (thisType.typeArguments.isEmpty) {
          val rootVName =
            emitter.nodeVName(generateSignature(thisType.typeSymbol), path)
          emitter.emitNodeKind(rootVName, NodeKind.RECORD)
          emitter.emitParamEdge(typeNode, rootVName, 0)
        } else {
          // polymorphic
          val rootVName = emitter.nodeVName(makeAbsSig(thisType), path)
          emitter.emitNodeKind(rootVName, NodeKind.ABS)
          emitter.emitParamEdge(typeNode, rootVName, 0)
        }

        for ((typeArg, index) <- thisType.typeArguments.zipWithIndex) {
          emitter.emitParamEdge(
            typeNode,
            makeNodeForType(typeArg, path),
            index + 1
          )
        }
      }

      /**
       * Decide if the type is complex or simple by checking typeArguments.
       * If complex, emit a tapp node and its subgraph
       * If simple, emit a type edge to the primitive or class node.
       */
      private def emitTypeEdgesForVariable(
          valdef: ValDef,
          sourceNode: emitter.NODE,
          path: String
      ): Unit = {
        // TODO(rbraunstein): look for aliases and try typeSymbolDirect instead o typeSymbol
        val typeNode = makeNodeForType(valdef.symbol.typeOfThis, path)
        emitter.emitEdge(sourceNode, EdgeKind.TYPED, typeNode)
        emitAnchorForType(valdef, typeNode)
      }

      /**
       * If a class is not parameterized, there is nothing to do.
       * If a class is parameterized, then we need to create a NodeKind.Abs tree.
       * i.e. for List[A]:
       * emit Abs node for List[A]
       * emit Absvar node for A
       * Link them with param.x edge
       */
      private def emitTypeEdgesForClassDef(
          symbol: Symbol,
          sourceNode: emitter.NODE,
          path: String
      ): Unit = {
        if (!symbol.isMonomorphicType) {
          val symbolType = symbol.typeOfThis

          val classTypeNode = emitter.nodeVName(makeAbsSig(symbolType), path)
          emitter.emitNodeKind(classTypeNode, NodeKind.ABS)
          emitter.emitEdge(sourceNode, EdgeKind.CHILD_OF, classTypeNode)
          for ((typeParam, index) <- symbol.typeParams.zipWithIndex) {
            val paramVName =
              emitter.nodeVName(generateSignature(typeParam), path)
            emitter.emitNodeKind(paramVName, NodeKind.ABSVAR)
            emitter.emitParamEdge(classTypeNode, paramVName, index)
          }
        }
      }

      /**
       * Abs nodes should have signature that is distinct from the class node signature.
       * Specifically, we try and add the param argument names, i.e. "List[A]", not "List"
       */
      def makeAbsSig(symbolType: Type) =
        s"scala_type:${symbolType.typeSymbol.typeOfThis.toLongString}"

      /**
       * Main interface for traversing the scala ast.
       */
      private case class DefinitionTraverser(fileNode: emitter.NODE)
          extends Traverser {
        // Always traverse the whole tree, but only emit anchors (links)
        // when we aren't somewhere beneath a synthetic scope.
        var isInsideSyntheticMethod = false
        var kytheScopeNode = fileNode

        /*
         * For the class, emit extends edges to any classes, interfaces or traits it
         * directly implements.
         */
        def emitExtendsEdges(
            tree: global.Tree,
            classVName: emitter.NODE
        ): Unit =
          for (parent <- tree.symbol.parentSymbols)
            emitter.emitEdge(
              classVName,
              EdgeKind.EXTENDS,
              emitter.nodeVName(
                generateSignature(parent),
                tree.pos.source.file.path
              )
            )

        /*
         * If this method overrides one in the parent class or trait, emit an override edges.
         * If it overrides some other ancestor, emit an override/transitive edge
         */
        def emitMethodOverrides(
            tree: global.Tree,
            methodVName: emitter.NODE
        ): Unit = {
          // If this class has traits, and this method is in those traits, it should
          // be an override
          val directParentClasses = tree.symbol.owner.parentSymbols
          for (orig <- tree.symbol.allOverriddenSymbols)
            if (orig != tree.symbol) // first member is self, need to ignore it.
              emitter.emitEdge(
                methodVName,
                if (directParentClasses.contains(orig.owner)) EdgeKind.OVERRIDES
                else EdgeKind.OVERRIDES_TRANSITIVE,
                emitter.nodeVName(
                  generateSignature(orig),
                  tree.pos.source.file.path
                )
              )
        }

        def withTraversalState[T](work: => T): Unit = {
          val oldSyntheticState = isInsideSyntheticMethod
          val oldScopeNode = kytheScopeNode
          try {
            work
          } catch {
            // Don't let an exception break the whole compile
            case e: Exception => {
              e.printStackTrace()
              emitter.emitFact(fileNode, FactName.MESSAGE, e.toString)
            }
          } finally {
            isInsideSyntheticMethod = oldSyntheticState
            kytheScopeNode = oldScopeNode
          }
        }

        override def traverse(tree: Tree): Unit = withTraversalState {
          tree match {
            // defs and decls
            case _: LabelDef => () // n/a
            case _: TypeDef =>
              emitter.emitAnchor(tree, EdgeKind.DEFINES_BINDING)
            case _: Bind => () // Ignore, causes problems for DefTree below
            case _: DefTree if (!tree.symbol.isAnonymousFunction) => {
              // Common for all Defs
              val vName = emitter.nodeVName(
                generateSignature(tree.symbol),
                tree.pos.source.file.path
              )
              kytheScopeNode = vName
              val kind = kytheType(tree)
              emitter.emitNodeKind(vName, kind)
              emitNameNode(tree, vName)
              emitTypeEdgesForDefinition(tree, vName)

              // emit anchors if we aren't in synthetic methods
              if (isNotGeneratedDef(tree) && !isInsideSyntheticMethod) {
                emitter.emitAnchor(tree, EdgeKind.DEFINES_BINDING)
                if (
                  tree.symbol
                    .isInstanceOf[ModuleSymbol] && !tree.symbol.hasPackageFlag
                ) {
                  emitter.emitAnchor(
                    tree,
                    tree.symbol.tpe.typeSymbol,
                    EdgeKind.DEFINES_BINDING
                  )
                }
                if (
                  !tree.symbol.hasFlag(
                    nsc.symtab.Flags.PACKAGE | nsc.symtab.Flags.MODULE
                  )
                )
                  emitter.emitAnchor(tree, EdgeKind.DEFINES)
              } else isInsideSyntheticMethod = true

              // Extra edges for each Def type
              tree match {
                case _: ClassDef => emitExtendsEdges(tree, vName)
                case defdef: DefDef => {
                  if (tree.symbol.accessedOrSelf.isVariable)
                    emitVariableExtras(defdef)
                  emitMethodOverrides(defdef, vName)
                }
                case vd: ValDef    => emitVariableExtras(vd)
                case _: ModuleDef  => ()
                case _: PackageDef => ()
              }
            }

            // edges for calls
            case Apply(sym, t) => {
              // don't emit Call edges for calls to implicit set routines.
              if (!sym.symbol.isSetter) {
                val anchor = emitRef(tree, EdgeKind.REF_CALL)
                if (anchor.isDefined)
                  emitter.emitEdge(
                    anchor.get,
                    EdgeKind.CHILD_OF,
                    kytheScopeNode
                  )
              }
            }

            // refs to fields, methods
            case _: Select => emitRef(tree, EdgeKind.REF)

            // We don't get AppliedTypeTrees unless we follow the 'original' tree in TypeTree
            case tt: TypeTree =>
              if (tt.original.isInstanceOf[AppliedTypeTree])
                traverse(tt.original)
            case AppliedTypeTree(tpt, args) =>
              emitRef(tpt, EdgeKind.REF)
              for (arg <- args) {
                emitRef(arg, EdgeKind.REF)
                traverse(arg)
              }

            case _: Ident => emitRef(tree, EdgeKind.REF)
            case TypeApply(_, args) =>
              for (arg <- args) {
                emitRef(arg, EdgeKind.REF)
              }

            case _ => ()
          }
          super.traverse(tree)
        }

        /**
         * Helper func for emitting anchors.
         * Does extra work to prevent references that aren't explicit in the original code.
         *
         * @return anchor node if available
         */
        def emitRef(tree: Tree, edgeKind: EdgeKind): Option[emitter.NODE] = {
          // Labels are weird.
          // For Packages we have trouble finding original binding.
          if (
            isNotGeneratedDef(tree)
            && !tree.symbol.isLabel
            && !tree.symbol.isPackageObjectOrClass
          ) {

            // For java, emit xrefs to name nodes, not VNames.
            emitter.emitAnchor(tree, edgeKind)
          } else None
        }

        // All definition nodes have a 'name' node as well.  For scala, it looks like
        // Try and use a mirror to generate the name instead of fullNameString.  That
        // should help with overloads.
        // NOTE: It is perfectly reasonable to not emit names for locals and params.
        def emitNameNode(tree: Tree, vName: emitter.NODE) = {
          val nodeVName = emitter.nodeVName(
            tree.symbol.fullNameString,
            ""
          )
          emitter.emitNodeKind(nodeVName, NodeKind.NAME)
          emitter.emitEdge(vName, EdgeKind.NAMED, nodeVName)
        }
      }

    }

  }

}

/* *
 * Add a main to make debugging in intellij easier.
 * This simply calls the normal scalac main and passes
 * the plugin jar as an option.
 *
 * You still need to setup the run configuration.
 * The two main thigs to set are:
 *   1) Program options, set the glob of files to analyze
 *   2) CLASSPATH env variable.
 *    CLASSPATH=$SCALA_HOME/lib/akka-actor_2.11-2.3.10.jar:$SCALA_HOME/lib/config-1.2.1.jar:$SCALA_HOME/lib/java_indexer.jar:$SCALA_HOME/lib/jline-2.12.1.jar:$SCALA_HOME/lib/scala-actors-2.11.0.jar:$SCALA_HOME/lib/scala-actors-migration_2.11-1.1.0.jar:$SCALA_HOME/lib/scala-compiler.jar:$SCALA_HOME/lib/scala-continuations-library_2.11-1.0.2.jar:$SCALA_HOME/lib/scala-continuations-plugin_2.11.8-1.0.2.jar:$SCALA_HOME/lib/scala-library.jar:$SCALA_HOME/lib/scalap-2.11.8.jar:$SCALA_HOME/lib/scala-parser-combinators_2.11-1.0.4.jar:$SCALA_HOME/lib/scala-reflect.jar:$SCALA_HOME/lib/scala-swing_2.11-1.0.2.jar:$SCALA_HOME/lib/scala-xml_2.11-1.0.4.jar
 * */
object ScalacWrapper extends scala.tools.nsc.Driver {
  override def newCompiler(): Global = {
    var s = settings.copy()
    // You have to create a .jar file with scalac-plugin.xml in it.
    // But it doesn't need any .class files.
    val myArgs = List("-Xplugin:kythe/scala/com/google/devtools/kythe/analyzers/scala/kythe-plugin.jar", "-Yrangepos")
    s.processArguments(myArgs, true) match {
      case (false, rest) => {
        println("error processing arguments (unprocessed: %s)".format(rest))
      }
      case _ => println("read args okay")
    }
    Global(s, reporter)
  }
}
