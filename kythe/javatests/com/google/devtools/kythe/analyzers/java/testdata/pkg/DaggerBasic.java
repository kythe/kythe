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

import dagger.Binds;
import dagger.Component;
import dagger.Module;
import dagger.Provides;
import javax.inject.Inject;

interface DaggerBasic {
  @Component(modules = BasicModule.class)
  interface Basic {
    //- @basicInject ref KeyBasicInject
    BasicInject basicInject();

    //- @basicProvides ref KeyBasicProvides
    BasicProvides basicProvides();

    //- @basicBinds ref KeyBasicBinds
    BasicBinds basicBinds();
  }

  @Module
  interface BasicModule {
    @Provides
    //- { @basicProvidesMethod defines/binding KeyBasicProvides =
    //-   vname("inject_key:pkg.DaggerBasic.BasicProvides", _, _, _, _)
    //- }
    //- @BasicProvides ref ClassBasicProvides
    static BasicProvides basicProvidesMethod() {
      //- KeyBasicProvides.node/kind inject/key
      //- KeyBasicProvides param.0 ClassBasicProvides
      //- ! { KeyBasicProvides param.1 _ }
      return new BasicProvides();
    }

    @Binds
    //- { @bind defines/binding KeyBasicBinds =
    //-   vname("inject_key:pkg.DaggerBasic.BasicBinds", _, _, _, _)
    //- }
    BasicBinds bind(BasicProvides basicProvides);
    //- KeyBasicBinds.node/kind inject/key
    //- KeyBasicBinds param.0 InterfaceBasicBinds
  }

  //- @BasicInject defines/binding ClassBasicInject
  class BasicInject {
    //- { @BasicInject defines/binding KeyBasicInject =
    //-   vname("inject_key:pkg.DaggerBasic.BasicInject", _, _, _, _)
    //- }
    @Inject BasicInject() {}
    //- KeyBasicInject.node/kind inject/key
    //- KeyBasicInject param.0 ClassBasicInject
  }

  //- @BasicProvides defines/binding ClassBasicProvides
  class BasicProvides implements BasicBinds {}

  //- @BasicBinds defines/binding InterfaceBasicBinds
  interface BasicBinds {}
}
