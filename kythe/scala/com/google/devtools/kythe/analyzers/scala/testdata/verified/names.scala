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
//  see kythe file: testdata/pkg/Names.java for more examples

//- File.node/kind file
//- File named vname("names.scala","","","","scala")

// TODO(rbraunstein): implement package bindings
//*- @pkg ref Package
//*- Package.node/kind package
//*- Package named vname("td","","","","scala")
package td;

// Checks that appropriate name nodes are emitted for simple cases

//- @Names defines/binding C
//- C named vname("td.Names","","","","scala")
object Names {
  // see java overloads here: java/testdata/pkg/Inheritance.java

  //- @overloaded defines/binding OverloadMeth1
  //- OverloadMeth1 named vname("td.Names.overloaded",_,_,_,"scala")
  //- !{ @overloaded defines/binding OverloadMeth2}
  //- !{ @overloaded defines/binding OverloadMeth3}
  //- @what defines/binding WhatParam1
  //- !{ @what defines/binding WhatParam2}
  def overloaded(what: String): String =
    what

  //- @overloaded defines/binding OverloadMeth2
  //- OverloadMeth2 named vname("td.Names.overloaded",_,_,_,"scala")
  //- !{ @overloaded defines/binding OverloadMeth1}
  //- !{ @overloaded defines/binding OverloadMeth3}
  //- @what defines/binding WhatParam2
  //- !{ @what defines/binding WhatParam1}
  def overloaded(what: String, where: Int): String = {
    val myLocal = 3
    //- @localFunc defines/binding LF1
    //- !{ @localFunc defines/binding LF2}
    def localFunc(): Unit = {
      //- @lf defines/binding LLF1
      //- !{ @lf defines/binding LLF2}
      val lf = 1
    }
    what
  }
  //- @overloaded defines/binding OverloadMeth3
  //- OverloadMeth3 named vname("td.Names.overloaded",_,_,_,"scala")
  //- !{ @overloaded defines/binding OverloadMeth1}
  //- !{ @overloaded defines/binding OverloadMeth2}
  def overloaded() = "boring"

  def overloaded(again: Int): String = {
    val myLocal = 7
    //- @localFunc defines/binding LF2
    //- !{ @localFunc defines/binding LF1}
    def localFunc(): Unit = {
      //- @lf defines/binding LLF2
      //- !{ @lf defines/binding LLF1}
      val lf = 2
    }
    //- @localFunc ref LF2
    //- !{ @localFunc ref LF1}
    localFunc
    myLocal.toString
  }

  def anons(): Int => Int = {
    // the anonVars below should be distinct
    if (true) {
      //- @anonVar defines/binding AnonVar1
      //- !{ @anonVar defines/binding AnonVar2}
      val anonVar = 1
    }
    if (true) {
      // TODO(rbraunstein): is there an easier way to say AnonVar1 != AnonVar2
      //- @anonVar defines/binding AnonVar2
      //- !{ @anonVar defines/binding AnonVar1}
      val anonVar = 2
      //- @anonVar ref AnonVar2
      //- !{ @anonVar ref AnonVar1}
      7 + anonVar
    }
    // useless anon func
    (x: Int) => x + 1
  }


  //- @defaultArg defines/binding OverLoadWithDef1
  //- !{@defaultArg defines/binding OverLoadWithDef2}
  //- !{@defaultArg defines/binding OverLoadWithDef3}
  def defaultArg(x: Int, y: Int = 0, z: Int) = x + y

  //- @defaultArg defines/binding OverLoadWithDef2
  //- !{@defaultArg defines/binding OverLoadWithDef1}
  //- !{@defaultArg defines/binding OverLoadWithDef3}
  def defaultArg(x: Int):Int =
  // TODO(rbraunstein): there is an extra anchor ref on defaultArg here
    defaultArg(x, z=0)

  //- @defaultArg defines/binding OverLoadWithDef3
  //- !{@defaultArg defines/binding OverLoadWithDef1}
  //- !{@defaultArg defines/binding OverLoadWithDef2}
  def defaultArg(): Int = 8
  //TODO(rbraunstein): more examples on implicit args and multiple param lists.
  // http://docs.scala-lang.org/sips/completed/named-and-default-arguments.html
}

// and more tests for names for things in anon scopes?
