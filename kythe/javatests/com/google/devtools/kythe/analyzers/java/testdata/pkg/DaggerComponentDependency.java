/*
 * Copyright 2017 Google Inc. All rights reserved.
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

package pkg;

import dagger.Component;

interface DaggerComponentDependency {
  //- { @Dep defines/binding KeyDep =
  //-   vname("inject_key:pkg.DaggerComponentDependency.Dep", _, _, _, _)
  //- }
  //- KeyDep.node/kind inject/key
  interface Dep {
    //- @object defines/binding KeyObject
    //- KeyObject.node/kind inject/key
    Object object();
  }

  @Component(dependencies = Dep.class)
  interface HasDependency {
    //- @objectThroughDependency ref KeyObject
    Object objectThroughDependency();

    //- @actualDepType ref KeyDep
    Dep actualDepType();
  }
}
