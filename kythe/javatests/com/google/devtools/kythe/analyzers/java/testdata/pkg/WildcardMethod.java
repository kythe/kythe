package pkg;


public final class WildcardMethod {
  public static final class Optional<T> {}

  public static final class Generic<T> {}

  //- @Optional ref OptionalClass
  //- @wildcard defines/binding WildcardFnAbs
  //- @ovar defines/binding WildcardParam1
  //- WildcardFnAbs.node/kind abs
  //- WildcardFnAbs param.0 Wildcard0
  //- WildcardFnDecl childof WildcardFnAbs
  //- WildcardFnDecl param.0 WildcardParam1
  //- WildcardParam1 typed WildcardParam1Type
  //- WildcardParam1Type.node/kind tapp
  //- WildcardParam1Type param.0 OptionalClass
  //- WildcardParam1Type param.1 Wildcard0
  //- Wildcard0.node/kind absvar
  //- WildcardFn childof WildcardFnAbs
  //- WildcardFn.node/kind function
  //- WildcardFn typed WildcardFnType
  //- WildcardFnType param.3 WildcardParam1Type
  //- !{ Wildcard0 bounded/upper Anything0
  //-    Wildcard0 bounded/lower Anything1 }
  //- !{ WildcardFnAbs param.1 _ }
  private static void wildcard(Optional<?> ovar) {}

  //- @#0Optional ref OptionalClass
  //- @#1Optional ref OptionalClass
  //- @wildcard2 defines/binding Wildcard2FnAbs
  //- @ovar defines/binding Wildcard2Param1
  //- @bvar defines/binding Wildcard2Param2
  //- Wildcard2FnAbs.node/kind abs
  //- Wildcard2FnAbs param.0 Wildcard2_0
  //- Wildcard2FnAbs param.1 Wildcard2_1
  //- Wildcard2FnDecl childof Wildcard2FnAbs
  //- Wildcard2FnDecl param.0 Wildcard2Param1
  //- Wildcard2FnDecl param.1 Wildcard2Param2
  //- Wildcard2Param1 typed WildcardParam2_1Type
  //- WildcardParam2_1Type.node/kind tapp
  //- WildcardParam2_1Type param.0 OptionalClass
  //- WildcardParam2_1Type param.1 Wildcard2_0
  //- Wildcard2_1.node/kind absvar
  //- Wildcard2Param2 typed WildcardParam2_2Type
  //- WildcardParam2_2Type.node/kind tapp
  //- WildcardParam2_2Type param.0 OptionalClass
  //- WildcardParam2_2Type param.1 Wildcard2_2
  //- Wildcard2_2.node/kind absvar
  //- !{ Wildcard2_1 bounded/upper Anything0
  //-    Wildcard2_1 bounded/lower Anything1 }
  //- !{ Wildcard2_2 bounded/upper Anything0
  //-    Wildcard2_2 bounded/lower Anything1 }
  private static void wildcard2(Optional<?> ovar, Optional<?> bvar) {}

  //- @Optional ref OptionalClass
  //- @wildcardBound defines/binding OptionalWildStringFnAbs
  //- @ovar defines/binding WBParam0
  //- OptionalWildStringFn childof OptionalWildStringFnAbs
  //- OptionalWildStringFn param.0 WBParam0
  //- WBParam0 typed OptionalWildString
  //- OptionalWildString.node/kind tapp
  //- OptionalWildString param.0 OptionalClass
  //- OptionalWildString param.1 Wildcard1
  //- Wildcard1.node/kind absvar
  //- Wildcard1 bounded/upper Str
  //- @String ref Str
  //- !{ Wildcard1 bounded/lower _ }
  //- !{ OptionalWildStringFnAbs param.1 _ }
  private static void wildcardBound(Optional<? extends String> ovar) {}

  //- @Optional ref OptionalClass
  //- @wildcardSuperBound defines/binding WildcardSuperBoundFnAbs
  //- @ovar defines/binding WSBParam0
  //- WildcardSuperBoundFn childof WildcardSuperBoundFnAbs
  //- WildcardSuperBoundFn param.0 WSBParam0
  //- WSBParam0 typed OptionalWildSuperString
  //- OptionalWildSuperString.node/kind tapp
  //- OptionalWildSuperString param.0 OptionalClass
  //- OptionalWildSuperString param.1 WildcardSuper1
  //- WildcardSuper1.node/kind absvar
  //- WildcardSuper1 bounded/lower Str
  //- !{ WildcardSuper1 bounded/upper Anything1 }
  //- @String ref Str
  private static void wildcardSuperBound(Optional<? super String> ovar) {}

  //- @Optional ref OptionalClass
  //- @Generic ref GenericClass
  //- @ogvar defines/binding OgVar
  //- OgVar typed OgType
  //- OgType.node/kind tapp
  //- OgType param.0 OptionalClass
  //- OgType param.1 FirstWildcard
  //- FirstWildcard.node/kind absvar
  //- FirstWildcard bounded/upper GType
  //- GType.node/kind tapp
  //- GType param.0 GenericClass
  //- GType param.1 SecondWildcard
  //- SecondWildcard.node/kind absvar
  //- @nestedWildcard defines/binding NestedWildFnAbs
  //- NestedWildFnAbs.node/kind abs
  //- NestedWildFnAbs param.0 FirstWildcard
  //- NestedWildFnAbs param.1 SecondWildcard
  //- !{ NestedWildFnAbs param.2 _ }
  //- !{ NestedWildFnAbs param.3 _ }
  private static void nestedWildcard(Optional<? extends Generic<?>> ogvar) {}

  //- @Optional ref OptionalClass
  //- @ovar defines/binding OVar
  //- OVar typed OType
  //- OType.node/kind tapp
  //- OType param.0 OptionalClass
  //- OType param.1 Wildcard
  //- Wildcard.node/kind absvar
  //- @tvar defines/binding TVar
  //- TVar typed TType
  //- TType.node/kind absvar
  //- @wildcardAndParam defines/binding WildcardAndParamFnAbs
  //- WildcardAndParamFnAbs.node/kind abs
  //- WildcardAndParamFnAbs param.0 TType
  //- WildcardAndParamFnAbs param.1 Wildcard
  //- !{ WildcardAndParamFnAbs param.2 _ }
  private static <T> void wildcardAndParam(Optional<?> ovar, T tvar) {}
}
