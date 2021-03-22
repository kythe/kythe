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
package testdata.verified2

/**
  * Created by rbraunstein on 5/31/16.
  */
class InheritsTest {

  // TODO(rbraunstein): Person doesn't extend anyting
  //- @Person defines/binding Person
  class Person(name: String)

  //- @Athlete defines/binding Athlete
  //- Athlete extends Person
  class Athlete(name: String) extends Person(name) {
    //- @play defines/binding PlayMeth
    def play(): Unit = ()
  }

  //- @Soccer defines/binding Soccer
  trait Soccer {
    def kick(): Unit
  }

  //- @Hockey defines/binding Hockey
  trait Hockey {
    def slap(): Unit
  }

  //- @Golf defines/binding Golf
  trait Golf {
    def swing(): Unit
    //- @waggle defines/binding TraitWaggle
    def waggle(): Int
  }

  //- @TriAthlete defines/binding Tri
  class TriAthlete(name: String) extends Athlete(name) with Soccer with Hockey with Golf {
    //- @play defines/binding TriPlayMeth
    override def play(): Unit = kick() ; slap()

    def kick() = ()
    def slap() = ()
    def swing() = ()
    //- @waggle defines/binding TriAthleteWaggle
    def waggle = 7
  }
  //- Tri extends Athlete
  //- Tri extends Golf
  //- Tri extends Hockey
  //- Tri extends Soccer

  //- @OldTriAthlete defines/binding OldTri
  class OldTriAthlete(name: String) extends TriAthlete(name) {
    //- @play defines/binding OldPlayMeth
    override def play(): Unit = {
      super.play
      swing
    }
    override def kick() = ()
    //- !{@waggle defines/binding TraitWaggle}
    //- !{@waggle defines/binding TriAthleteWaggle}
    override def waggle = 13
    override def toString = name
  }
  //- OldTri extends Tri
  //- !{OldTri extends Athlete}
  //- !{OldTri extends Golf}
  //- !{OldTri extends Person}

  //- OldPlayMeth overrides TriPlayMeth
  //- TriPlayMeth overrides PlayMeth

  //
 }
