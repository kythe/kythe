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
object functions {
  //- @field defines/binding Field_f1
  //- Field_f1.node/kind variable
  //- Field_f1.subkind field
  val field = 7

  //- @Null defines/binding NullFunc
  //- NullFunc.node/kind function
  def Null(): Unit = { }

  //- @ParamsAndLocals defines/binding PAL
  //- PAL.node/kind function
  //- @arg1 defines/binding Arg1
  //- Arg1.node/kind variable
  //- Arg1.subkind local/parameter
  def ParamsAndLocals(arg1: String, arg2: String): Unit = {

    //- @local defines/binding LOCAL
    //- LOCAL.node/kind variable
    //- LOCAL.subkind local
    var local = "seven"
    local = arg1
  }

  //
  // TODO(rbraunstein): Add verifier controls for ideas from Joshua:
  //  especially on overloads.
  //     curried parameter lists, e.g.

  //- @n1 defines/binding VarN1
  //- @x1 defines/binding VarX1
  //- VarN1.node/kind variable
  //- VarX1.node/kind variable
  //- VarX1.subkind local/parameter
  //- ModN param.0 VarN1
  //- ModN param.1 VarX1
  def modN(n1: Int)(x1: Int) =
    ((x1 % n1) == 0)

  def funFunc(arg1: String)(arg2: String) = ()
  // Additionally, marking which parameter list is the implciit one, e.g.
  def funFunc(normal: String)(implicit arg1: String, arg2: Int) = ()
}
