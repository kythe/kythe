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
import dagger.Module;
import dagger.Provides;
import javax.inject.Inject;

interface DaggerDependencies {
  @Component(modules = DependenciesModule.class)
  interface Dependencies {
    //- @injectWithDep ref KeyInjectWithDep
    InjectWithDep injectWithDep();

    //- @providesWithDep ref KeyProvidesWithDep
    ProvidesWithDep providesWithDep();
  }

  class InjectWithDep {
    //- @InjectWithDep defines/binding KeyInjectWithDep
    //- KeyInjectWithDep.node/kind inject/key
    //- @dep ref KeyBasicInject
    @Inject InjectWithDep(BasicInject dep) {}
  }

  class ProvidesWithDep {}

  @Module
  interface DependenciesModule {
    @Provides
    //- @providesWithDep defines/binding KeyProvidesWithDep
    //- KeyProvidesWithDep.node/kind inject/key
    //- @dep ref KeyInjectWithDep
    static ProvidesWithDep providesWithDep(InjectWithDep dep) {
      return new ProvidesWithDep();
    }
  }

  class BasicInject {
    //- @BasicInject defines/binding KeyBasicInject
    @Inject BasicInject() {}
  }
}
