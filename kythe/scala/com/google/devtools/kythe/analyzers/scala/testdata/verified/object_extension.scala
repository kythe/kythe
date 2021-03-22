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
 * The main goal here is to prevent overlapping links
 * that we see in our own code (i.e. NodeKind.scala)
 * Currenttly at "Abs", we see 1 definition: good
 * and 5 references, not so good
 * i.e. For this definition:
 *   case object Abs extends NodeKind("abs")
 * we create references to these 5 methods:
 *  "java.lang.IndexOutOfBoundsException.<init>"
 *  "scala.Any.toString"
 *  "scala.runtime.ScalaRunTime.typedProductIterator"
 *  "scala.runtime.ScalaRunTime"
 *  "com.google.devtools.codeindex.indexers.scala.NodeKind.Abs"
 *
 * At Kind, we see 2 anchors, one ref for Kind and one decl for Kind.<init>
 * We want to ensure there is no <init> method
 *
 * I think it is okay to callgraph entries, but we don't want to have links
 * for people to click on.
 */

//- @Kind defines/binding KindClass
sealed abstract class Kind(val kind: String)

/**
 * Okay, this verifier dsl is so magical, its hard to
 * read to figure out what it is trying to say.
 * see //third_party/kythe/kythe/cxx/indexer/cxx/testdata/rec/rec_implicit_anchors.cc
 *
 * We are verifying that the symbol abs
 * defines a new class.
 * But more importantly, we are ensuring that all the references
 * to methods inside the generated functions don't show up as anchors.
 *
 * At the string "Kind" in "extends Kinds", we ensure there is an anchor
 * that refers to an earlier binding.
 * We also ensure that at "kind", there are no anchors for new bindings.
 *
 * */
object Kind {
  //- @Kind ref KindClass
  //- SingleRefAnchor?=@Kind.node/kind anchor
  //- SingleRefAnchor.loc/start @^K
  //- SingleRefAnchor.loc/end @$Kind
  case object Abs extends Kind("abs")

}

