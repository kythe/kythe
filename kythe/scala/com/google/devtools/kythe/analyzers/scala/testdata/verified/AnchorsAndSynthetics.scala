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
/*
 * We want to only emit an anchor when there is user code, not
 * synthetic code.  'case class' objects give us lots of synthetic
 * code where anchors don't make sense.
 * But there are also elements we miss:
 */

class AnchorsAndSynthetics {

  def getPoint(): Unit = {
    //- @z defines/binding VarZ
    val z = 10
    //- @i defines/binding VarI
    var i = 1

    //- @x defines/binding VarX
    //- @y defines/binding VarY
    val (x, y) = {
      //- @z ref VarZ
      if (z > 5) (i, i + 1)
      //- @i ref VarI
      else (z, i -1)
    }

    //- @x ref VarX
    if (x == 8) {
      //- @y ref VarY
      i = y
    }
  }
}
