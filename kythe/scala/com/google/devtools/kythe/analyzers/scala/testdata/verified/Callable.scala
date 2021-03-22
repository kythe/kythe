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

/*
 * Tests for making sure we emit ref/call and childof for callgraphs
 * The main interesting points are anonymous methods, implicit method calls
 * and calls of synthetic methods.
 * Calls to java code are interesting code.
 */
class Callable {

  //- @CallMe defines/binding CM
  def CallMe(): Unit = ()
  def Anytime(): Unit = {
    System.out.println("fred")
    CallMe()
  }

  val Funny = (x:Int) => x + 3
  val Bunny = (x: Int) => Funny(2* x)

  //- @Harvey defines/binding MethodHarvey
  def Harvey(z: Int): Int = {
    //- @"CallMe()" ref/call CM
    //- @"CallMe()" childof MethodHarvey
    CallMe()
    Funny(
      Bunny(7))
  }

  ((x:Int) => x+1)(2)

  def m1(x:Int) = x+3
  val f1 = (x:Int) => x+3
  def m2[T](x:T) = x.toString.substring(0,4)
}
