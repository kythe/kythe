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
import dagger.Module;
import dagger.Subcomponent;
import javax.inject.Inject;

interface DaggerSubcomponents {
  @Subcomponent
  interface Sub {
    @Subcomponent.Builder
    interface Builder {
      Sub build();
    }
  }

  @Component(modules = InstallsSubcomponentModule.class)
  interface SubcomponentBuilderBinding {
    InjectsSubBuilder injectsSubBuilder();
  }

  class InjectsSubBuilder {
    //- @subBuilder ref KeySubBuilder
    @Inject InjectsSubBuilder(Sub.Builder subBuilder) {}
  }

  //- { @subcomponents defines/binding KeySubBuilder =
  //-   vname("inject_key:pkg.DaggerSubcomponents.Sub.Builder", _, _, _, _)
  //- }
  //- KeySubBuilder.node/kind inject/key
  @Module(subcomponents = Sub.class)
  interface InstallsSubcomponentModule {}

  @Component
  interface HasSubcomponentBuilderMethod {
    //- @subcomponentBuilderMethod ref KeySubBuilder
    Sub.Builder subcomponentBuilderMethod();
  }
}
