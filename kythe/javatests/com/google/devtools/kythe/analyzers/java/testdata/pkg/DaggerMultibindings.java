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

import static java.lang.annotation.RetentionPolicy.RUNTIME;

import dagger.Component;
import dagger.Module;
import dagger.Provides;
import dagger.multibindings.ElementsIntoSet;
import dagger.multibindings.IntoSet;
import dagger.multibindings.Multibinds;
import java.lang.annotation.Retention;
import java.util.HashSet;
import java.util.Set;
import javax.inject.Qualifier;

interface DaggerMultibindings {
  @Component(modules = MultibindingsModule.class)
  interface Multibindings {
    //- @emptySet ref KeySetOfString
    Set<String> emptySet();

    //- @numberSet ref KeySetOfNumber
    Set<Number> numberSet();

    //- @qualifiedNumberSet ref KeyQualifiedSetOfNumber
    @SimpleQualifier Set<Number> qualifiedNumberSet();
  }

  @Module
  interface MultibindingsModule {
    @Multibinds
    //- { @stringSet defines/binding KeySetOfString =
    //-   vname("inject_key:java.util.Set<java.lang.String>", _, _, _, _)
    //- }
    //- KeySetOfString.node/kind inject/key
    Set<String> stringSet();

    @Multibinds
      //- @secondMultibindsMethod defines/binding KeySetOfString
    Set<String> secondMultibindsMethod();

    @Provides
    @IntoSet
    //- { @contribution1 defines/binding KeySetOfNumber =
    //-   vname("inject_key:java.util.Set<java.lang.Number>", _, _, _, _)
    //- }
    static Number contribution1() {
      //- KeySetOfNumber.node/kind inject/key
      //- KeySetOfNumber param.0 ClassSet
      return 1;
    }

    @Provides
    @IntoSet
    //- @contribution2 defines/binding KeySetOfNumber
    static Number contribution2() {
      return 2;
    }

    @Provides
    @ElementsIntoSet
    //- @elementsIntoSet defines/binding KeySetOfNumber
    static Set<Number> elementsIntoSet() {
      return new HashSet<>();
    }

    @Provides
    @IntoSet
    @SimpleQualifier
    //- { @qualifiedContribution defines/binding KeyQualifiedSetOfNumber =
    //-   vname("inject_key:@pkg.DaggerMultibindings.SimpleQualifier java.util.Set<java.lang.Number>", _, _, _, _)
    //- }
    static Number qualifiedContribution() {
      //- KeyQualifiedSetOfNumber.node/kind inject/key
      //- KeyQualifiedSetOfNumber param.0 ClassSet
      //- KeyQualifiedSetOfNumber param.1 SimpleQualifierUsage
      return 1;
    }
  }

  // @Set ref ClassSet
  void method(Set s);

  @Qualifier
  @Retention(RUNTIME)
  @interface SimpleQualifier {}
}
