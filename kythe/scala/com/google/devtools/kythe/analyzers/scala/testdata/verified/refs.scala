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

class refs {
  //- @x defines/binding VarX
  //- VarX.node/kind variable 
  //- AnchorAtVarX defines/binding vname(_,_,_,_,_)
  //- AnchorAtVarX.loc/start @^x
  //- !{AnchorAtVarX refs vname("refs.x_=",_,_,_,_)}
  var x = 3
  def func1(): Int = {
    //- @x ref VarX
    x = 5
    //- @x ref VarX
    2 * x
  }
  def func2(): Int = {
    //- @x ref VarX
    x = func1() + 3
    //x = 8 + funFunc("7", 10)
    funFunc("3",3)
    x
  }

  // see testdata/function/function_args_defn.cc
  //- @funFunc defines/binding FuncWithArgs
  //- @arg1 defines/binding StringArg1
  //- @arg2 defines/binding vname(_,_,_,_,_)
  //- FuncWithArgs param.0 StringArg1
  //- FuncWithArgs param.1 vname(_,_,_,_,_)
  def funFunc(arg1: String, arg2: Int) = {
    //- @arg1 ref StringArg1
    println(arg1)
  }
}

