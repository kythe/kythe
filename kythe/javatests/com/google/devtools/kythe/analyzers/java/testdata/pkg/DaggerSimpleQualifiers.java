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
import javax.inject.Qualifier;

interface DaggerSimpleQualifiers {
  //- @BasicProvides defines/binding ClassBasicProvides
  class BasicProvides {}

  @Component(modules = QualifiersModule.class)
  interface Qualifiers {
    //- @qualifiedBasicProvides ref KeyQualified
    @SimpleQualifier BasicProvides qualifiedBasicProvides();
  }

  @Module
  interface QualifiersModule {
    @Provides
    @SimpleQualifier
    //- { @qualifiedBasicProvides defines/binding KeyQualified =
    //-   vname("inject_key:@pkg.DaggerSimpleQualifiers.SimpleQualifier pkg.DaggerSimpleQualifiers.BasicProvides", _, _, _, _)
    //- }
    static BasicProvides qualifiedBasicProvides() {
      //- KeyQualified.node/kind inject/key
      //- KeyQualified param.0 ClassBasicProvides
      //- { KeyQualified param.1 QualifierUsage =
      //-   vname("inject_qualifier:@pkg.DaggerSimpleQualifiers.SimpleQualifier", _, _, _, _)
      //- }
      //- QualifierUsage.node/kind inject/qualifier
      return new BasicProvides();
    }
  }

  @Qualifier
  @interface SimpleQualifier {}
}
