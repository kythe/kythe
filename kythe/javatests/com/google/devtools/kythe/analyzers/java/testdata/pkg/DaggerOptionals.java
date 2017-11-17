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

import dagger.BindsOptionalOf;
import dagger.Component;
import dagger.Module;
import dagger.Provides;

interface DaggerOptionals {
  interface Present {}
  interface Absent {}

  @Component(modules = OptionalsModule.class)
  interface Optionals {
    //- @present ref KeyOptionalOfPresent
    java.util.Optional<Present> present();

    //- @absent ref KeyOptionalOfAbsent
    java.util.Optional<Absent> absent();
  }

  @Module
  interface OptionalsModule {
    @Provides
    static Present providePresent() { return new Present(){}; }

    //- { @present defines/binding KeyOptionalOfPresent =
    //-   vname("inject_key:java.util.Optional<pkg.DaggerOptionals.Present>", _, _, _, _)
    //- }
    //- KeyOptionalOfPresent.node/kind inject/key
    @BindsOptionalOf Present present();

    //- { @absent defines/binding KeyOptionalOfAbsent =
    //-   vname("inject_key:java.util.Optional<pkg.DaggerOptionals.Absent>", _, _, _, _)
    //- }
    //- KeyOptionalOfAbsent.node/kind inject/key
    @BindsOptionalOf Absent absent();
  }
}
