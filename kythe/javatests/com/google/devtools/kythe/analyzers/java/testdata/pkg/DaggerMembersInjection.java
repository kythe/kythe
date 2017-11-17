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
 *
 *
 */

package pkg;

import dagger.Component;
import dagger.MembersInjector;
import javax.inject.Inject;

interface DaggerMembersInjection {
  class BasicInject {
    //- @BasicInject defines/binding KeyBasicInject
    //- KeyBasicInject.node/kind inject/key
    @Inject BasicInject() {}
  }

  // The MembersInjector<T> binding defines the binding at the type, since an @Inject constructor
  // may not be present:
  //- @InjectMyMembers defines/binding KeyInjectMyMembers
  class InjectMyMembers {
    // The @Inject binding is for a fully-constructed binding, at the constructor
    //- @InjectMyMembers defines/binding KeyInjectMyMembers
    @Inject InjectMyMembers() {}

    //- @field ref KeyBasicInject
    @Inject BasicInject field;

    //- @param1 ref KeyBasicInject
    //- @param2 ref KeyBasicInject
    @Inject void method(BasicInject param1, BasicInject param2) {}
  }

  @Component
  interface MembersInjection {
    //- { @returnsInjector ref KeyInjectMyMembers =
    //-   vname("inject_key:pkg.DaggerMembersInjection.InjectMyMembers", _, _, _, _)
    //- }
    //- KeyInjectMyMembers.node/kind inject/key
    MembersInjector<InjectMyMembers> returnsInjector();

    //- @fullyInjected ref KeyInjectMyMembers
    InjectMyMembers fullyInjected();

    //- @injectMethod ref KeyInjectMyMembers
    void injectMethod(InjectMyMembers instance);
  }
}
