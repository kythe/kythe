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

import scala.collection.mutable

// TODO: Type bounds (e.g. we don't handle <A extends B>

trait AddSigs {
  val global: scala.tools.nsc.Global
  import global._

  // Map to store signature -> symbol.
  // If we detect that a symbol generates a signature but doesn't already exist within the map, we
  // need to generate a new signature for it
  // This helps us deal with anonymous variables reusing names in separate blocks.
  // It's not as clean as Java's BlockAnonymousSignatureGenerator but it does the trick.
  private val anonymousMap: mutable.Map[String, mutable.LinkedHashSet[Symbol]] =
    mutable.Map.empty

  private case class Context(boundTypeParameters: Set[Symbol])

  private def scalaTypeToJavaType(
      classSymbol: ClassSymbol,
      context: Context
  ): Option[String] = {
    classSymbol.javaBinaryName.toString match {
      case "scala/Array" =>
        Some(
          "[" + generateSignature(
            classSymbol.typeParams.headOption.get,
            context
          )
        )
      case "scala/Byte"    => Some("B")
      case "scala/Char"    => Some("C")
      case "scala/Short"   => Some("S")
      case "scala/Int"     => Some("I")
      case "scala/Long"    => Some("J")
      case "scala/Float"   => Some("F")
      case "scala/Double"  => Some("D")
      case "scala/Boolean" => Some("Z")
      case "scala/Unit"    => Some("V")
      case _               => None
    }
  }

  /**
   * Key function for kythe.
   * We generate a signature for each node.
   * Rather than rely on 'fullNameString' in Symbol, we generate a scoped name
   * that includes types for method params.  Walking the owner chain allows us to
   * disambiguate overloaded methods.
   */
  def generateMethodSymbol(m: MethodSymbol, context: Context): String = {
    return generateSignature(m.owner, context) +
      "." +
      m.name.toString +
      ":(" +
      m.toType.params
        .map(_.toType)
        .map(_.typeSymbol)
        .map(param => generateSignature(param, context))
        .mkString(
          ""
        ) + // Java doesn't put commas between method parameter types.
      ")" +
      generateSignature(m.returnType.typeSymbol, context)
  }

  private def generateSignature(symbol: Symbol, context: Context): String = {
    symbol match {
      case c: ClassSymbol        => generateClassSignature(c, context)
      case m: MethodSymbol       => generateMethodSymbol(m, context)
      case m: ModuleSymbol       => generateModuleSignature(m)
      case t: TermSymbol         => generateVarSignature(t, context)
      case t: AbstractTypeSymbol => generateTypeSymbol(t, context)
      case t: AliasTypeSymbol    => generateAliasTypeSymbol(t, context)
      case t: TypeSkolem         => generateSignature(t.originalOwner, context)
      case _: NoSymbol           => ""
    }
  }

  def generateSignature(symbol: Symbol): String = {
    val signature = generateSignature(symbol, Context(Set()))
    val existingSymbols =
      anonymousMap.getOrElseUpdate(signature, new mutable.LinkedHashSet[Symbol])
    if (!existingSymbols.contains(symbol)) {
      existingSymbols += symbol
    }
    signature + "#" + existingSymbols.toList.indexOf(symbol)
  }

  def generateModuleSignature(moduleSymbol: ModuleSymbol): String = {
    return moduleSymbol.javaBinaryName.toString
  }

  /**
   * Assembles the signature of the class. Use if you know it's not a primitive type (check with
   * scalaTypeToJavaType)
   */
  private def assembleClassSignature(
      classSymbol: ClassSymbol,
      context: Context
  ): String = {
    val className: String = classSymbol.owner match {
      case _: PackageClassSymbol => classSymbol.javaBinaryName.toString
      case symbol: ClassSymbol =>
        assembleClassSignature(
          symbol,
          context
        ) + "$" + classSymbol.javaBinaryName
      case _ =>
        classSymbol.javaBinaryName.toString
    }
    if (classSymbol.typeParams.nonEmpty) {
      className +
        "<" +
        classSymbol.typeParams
          .map(typeParam =>
            generateSignature(
              typeParam,
              Context(
                context.boundTypeParameters ++ classSymbol.typeParams
              )
            )
          )
          .mkString(",") +
        ">"
    } else {
      className
    }
  }

  def generateClassSignature(
      classSymbol: ClassSymbol,
      context: Context
  ): String = {
    return scalaTypeToJavaType(classSymbol, context)
      .getOrElse("L" + assembleClassSignature(classSymbol, context) + ";")
  }

  def generateTypeSymbol(
      symbol: AbstractTypeSymbol,
      context: Context
  ): String = {
    // If we're just trying to describe a generic type, just return the name of the generic type.
    // Example: class Example[T] // <-- T is the symbol
    return if (context.boundTypeParameters.contains(symbol)) {
      // T<typename>; is how java generates it.
      "T" + symbol.name.toString + ";"
    } else {
      // Here we're trying to reference the generic type defined in our owner.
      // Example: class Example[T] {
      //   var t: T // <-- Here T is the symbol but we're referencing the one above.
      // }
      generateSignature(
        symbol.owner,
        Context(context.boundTypeParameters + symbol)
      ) + "~" + symbol.name.toString
    }
  }

  def generateVarSignature(termSymbol: TermSymbol, context: Context): String = {
    generateSignature(termSymbol.owner, context) +
      "." +
      termSymbol.javaSimpleName.toString +
      ":" +
      generateSignature(termSymbol.toType.typeSymbol, context)
  }

  /*
   * Create a signature for the instantiated type.
   *  i.e. if passed List[Int], we need to return List[Int], not List[A]
   */
  def makeSigForTappNode(symbolType: Type): String = {
    s"scala_type_app:${symbolType.toLongString}"
  }

  def generateAliasTypeSymbol(
      symbol: AliasTypeSymbol,
      context: Context
  ): String = {
    symbol.owner match {
      case _: PackageClassSymbol => symbol.javaBinaryName.toString
      case c: ClassSymbol =>
        generateClassSignature(c, context) +
          "." +
          symbol.simpleName.toString +
          ":" +
          generateSignature(symbol.info.typeSymbol, context)
      case _ => symbol.javaBinaryName.toString
    }
  }

}
