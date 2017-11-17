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

interface DaggerComplexQualifiers {
  //- @BasicProvides defines/binding ClassBasicProvides
  class BasicProvides {}

  @Component(modules = QualifiersModule.class)
  interface Qualifiers {
    //- @complexQualified ref KeyComplexQualified
    @ComplexQualifier(i = 1, str = "abc") BasicProvides complexQualified();

    //- @usesDefaults ref KeyUsesDefaults
    @ComplexQualifier(str = "uses defaults") BasicProvides usesDefaults();

    //- @jumbledAttributeOrder ref KeyJumbledOrder
    @ComplexQualifier(str = "abc", i = 1) String jumbledAttributeOrder();

    //- @hasQuoteInString ref KeyHasQuoteInString
    @ComplexQualifier(str="\"") String hasQuoteInString();

    //- @nestedQualifier ref KeyNestedQualifier
    @Nesting(nested = @ComplexQualifier(str = "abc", i = 1)) String nestedQualifier();
  }

  @Module
  interface QualifiersModule {
    @Provides
    //- { @complexQualified defines/binding KeyComplexQualified =
    //-   vname("inject_key:@pkg.DaggerComplexQualifiers.ComplexQualifier(i=1, str=\"abc\") pkg.DaggerComplexQualifiers.BasicProvides", _, _, _, _)
    //- }
    static @ComplexQualifier(i = 1, str = "abc") BasicProvides complexQualified() {
      //- KeyComplexQualified.node/kind inject/key
      //- KeyComplexQualified param.0 ClassBasicProvides
      //- { KeyComplexQualified param.1 ComplexQualifierUsage =
      //-   vname("inject_qualifier:@pkg.DaggerComplexQualifiers.ComplexQualifier(i=1, str=\"abc\")", _, _, _, _)
      //- }
      //- ComplexQualifierUsage.node/kind inject/qualifier
      return new BasicProvides();
    }

    @Provides
    @ComplexQualifier(str = "uses defaults")
    //- { @usesDefaults defines/binding KeyUsesDefaults =
    //-   vname("inject_key:@pkg.DaggerComplexQualifiers.ComplexQualifier(i=-1, str=\"uses defaults\") pkg.DaggerComplexQualifiers.BasicProvides", _, _, _, _)
    //- }
    static BasicProvides usesDefaults() {
      return new BasicProvides();
    }
    //- KeyUsesDefaults.node/kind inject/key
    //- KeyUsesDefaults param.0 ClassBasicProvides
    //- { KeyUsesDefaults param.1 ComplexQualifierUsesDefaults =
    //-   vname("inject_qualifier:@pkg.DaggerComplexQualifiers.ComplexQualifier(i=-1, str=\"uses defaults\")", _, _, _, _)
    //- }
    @Provides
    @ComplexQualifier(str = "abc", i = 1)
    //- @jumbledOrder defines/binding KeyJumbledOrder
    static String jumbledOrder() {
      //- KeyJumbledOrder.node/kind inject/key
      //- KeyJumbledOrder param.1 ComplexQualifierUsage
      return "";
    }

    @Provides
    @ComplexQualifier(str="\"")
    //- { @hasQuoteInString defines/binding KeyHasQuoteInString =
    //-   vname("inject_key:@pkg.DaggerComplexQualifiers.ComplexQualifier(i=-1, str=\"\\\"\") java.lang.String", _, _, _, _)
    //- }
    static String hasQuoteInString() {
      //- KeyHasQuoteInString.node/kind inject/key
      return "";
    }

    @Provides
    @Nesting(nested = @ComplexQualifier(str = "abc", i = 1))
    //- { @nested defines/binding KeyNestedQualifier =
    //-   vname("inject_key:@pkg.DaggerComplexQualifiers.Nesting(nested=@pkg.DaggerComplexQualifiers.ComplexQualifier(i=1, str=\"abc\")) java.lang.String", _, _, _, _)
    //- }
    static String nested() {
      //- KeyNestedQualifier.node/kind inject/key
      return "";
    }
  }

  @Qualifier
  @interface ComplexQualifier {
    int i() default -1;
    String str();
  }

  @Qualifier
  @interface Nesting {
    ComplexQualifier nested();
  }
}
