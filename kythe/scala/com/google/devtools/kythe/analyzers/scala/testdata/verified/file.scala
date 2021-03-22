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
package test
// # sig, corpus, root, path, lang
//- File?.node/kind file
//- vname("", "", "", "kythe/scala/com/google/devtools/kythe/analyzers/scala/testdata/verified/file.scala", "")
//-   .node/kind file
//- VerifiedFileBinding.node/kind record
//- @VerifiedFile defines/binding VerifiedFileBinding
class VerifiedFile {
  //- @VerifiedFile ref VerifiedFileBinding
  var testFile: VerifiedFile = null
}
