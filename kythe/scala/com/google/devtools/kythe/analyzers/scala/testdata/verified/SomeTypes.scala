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

package testdata.verified

// Just define a couple base types and create Def evars for later use
//- @Foo defines/binding FooClassDef
//- FooClassDef.node/kind record
case class Foo() {}
//- @Blah defines/binding BlahClassDef
case object Blah {}

//  /- @"SomeTypes[A]" defines/binding Type_STa  -- NO anchor for this, yet
//- @SomeTypes defines/binding RawClass
//- RawClass.node/kind record
//- RawClass childof TypeParameterizedClass
//- TypeParameterizedClass.node/kind abs
//- TVar.node/kind absvar
//- TypeParameterizedClass param.0 TVar
//- @A defines/binding TVar
abstract class SomeTypes[A] {

  // TODO(rbraunstein): Field Types aren't verified yet
  // ideally, we have a type for the field, but the field doesn't
  // realy exist in the final code, only the accessor methods.
  var ignoreForNow = List("field", "accessors", "become", "methods")

  //  TODO(rbraunstein): //- @A ref TVar
  //- @a defines/binding TypeParamVar
  //- TypeParamVar typed TVar
  def fun1(a: A): String = {
    //- @x defines/binding LocalX
    //- LocalX typed TVar
    val x = a
    x.toString + ignoreForNow.toString
  }

  def paramTypes(arg1: Map[String, List[Int]],
  //- @String ref StringDef
                 arg2: Tuple2[Tuple3[A, String, Foo], AnyRef])
       : Integer = {
         8
  }
  //  TODO(rbraunstein): @"List[String]" defines/binding TypeListOfString
  //                                     or ref
  def wrapper(): Unit = {
    val one = 1
    // Ensure the two List refs point to the same thing.
    //  -  ListAbsDef named vname(".*immutable::List",_,_,_,_)
    //  - StringDef named vname("String",_,_,_,_)
    // first)
    //- @things defines/binding VarThings
    //- VarThings typed TypeListOfString
    //- TypeListOfString.node/kind tapp
    //- TypeListOfString param.0 ListAbsDef
    //- TypeListOfString param.1 StringDef
    //- ListAbsDef.node/kind abs
    //- StringDef.node/kind record
    //- VarThings.node/kind variable
    var things: List[String] = List("ron")

    // NOTE: TypeListOfInt of here is the same as used inside the Map
    //       for bdays below.
    // NOTE: The ListAbsDef here (pointed to by the tapp node) is the same as above in things
    //       even though one is a List[String], and the other is List[Int]
    //- @nums defines/binding VarNums
    //- VarNums typed TypeListOfInt
    //- TypeListOfInt.node/kind tapp
    //- TypeListOfInt param.0 ListAbsDef
    var nums: List[Int] = List(7,8,9)

    //- @bdays defines/binding VarBdays
    //- VarBdays typed TypeMapStringToListOfInt
    //- TypeMapStringToListOfInt.node/kind tapp
    //- TypeMapStringToListOfInt param.0 MapDef
    //- TypeMapStringToListOfInt param.1 StringDef
    //- TypeMapStringToListOfInt param.2 TypeListOfInt
    //- MapDef.node/kind abs
    var bdays: Map[String, List[Int]] = Map(("May", List(1,2,3)))

    // Some simpler types.
    //- @sysout defines/binding VarOut
    //- VarOut typed SystemOutDef
    //- SystemOutDef.node/kind record
    val sysout = System.out

    // See above for FooDef and BarDef
    //- @foo defines/binding VarFoo
    //- VarFoo typed FooClassDef
    val foo = Foo()

    //- @blah defines/binding VarBlah
    //- VarBlah typed TypeListOfBlah
    //- TypeListOfBlah param.0 ListAbsDef
    //- TypeListOfBlah param.1 BlahClassDef
    val blah = List(Blah)

    // Strings are classes, but Ints are builtins.

    //- @me defines/binding VarMe
    //- VarMe typed StringDef
    //- StringDef.node/kind record
    val me = "ron"

    //- @num defines/binding VarNum
    //- VarNum typed Int
    //- Int.node/kind tbuiltin
    val num = Int.MinValue
    val integer = Int.box(one)
    //- @hungry defines/binding VarHungry
    //- VarHungry typed TypeBool
    //- TypeBool.node/kind tbuiltin
    val hungry = true
  }
}
