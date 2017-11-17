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
import dagger.Provides;
import javax.inject.Inject;

interface DaggerParameterized {
  @Component(modules = GenericsModule.class)
  interface Generics {
    //- @ofString ref KeyParameterizedOfString
    Parameterized<String> ofString();
    //- @ofNumber ref KeyParameterizedOfNumber
    Parameterized<Number> ofNumber();
    //- @ofWildcard ref KeyParameterizedOfWildcard
    Parameterized<?> ofWildcard();
  }

  class Parameterized<T> {
    //- @Parameterized defines/binding KeyParameterizedOfString
    //- @Parameterized defines/binding KeyParameterizedOfNumber
    //- KeyParameterizedOfString.node/kind inject/key
    //- KeyParameterizedOfNumber.node/kind inject/key
    @Inject Parameterized() {}
  }

  @Module
  interface GenericsModule {
    @Provides
    //- @wildcard defines/binding KeyParameterizedOfWildcard
    //- KeyParameterizedOfWildcard.node/kind inject/key
    static Parameterized<?> wildcard() {
      return new Parameterized<Object>();
    }
  }
}
