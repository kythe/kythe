package pkg;

@SuppressWarnings("unused")
public final class GenericMethod {
  public static final class Optional<T> {}

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
  //- !{ Wildcard0 bounded/upper Anything0
  //-    Wildcard0 bounded/lower Anything1 }
  private static void wildcard(Optional<?> ovar) {}

  //- @#0T defines/binding TVWVar
  //- @verboseWildcard defines/binding VerboseWildcardFnAbs
  //- @ovar defines/binding VWParam1
  //- TVWVar.node/kind absvar
  //- VerboseWildcardFnAbs.node/kind abs
  //- VerboseWildcardFnAbs param.0 TVWVar
  //- VerboseWildcardFn childof VerboseWildcardFnAbs
  //- VerboseWildcardFn.node/kind function
  //- VerboseWildcardFn param.0 VWParam1
  //- VWParam1 typed VWParam1Type
  //- VWParam1Type.node/kind tapp
  //- VWParam1Type param.0 OptionalClass
  //- VWParam1Type param.1 TVWVar
  //- !{ TVWVar bounded/upper.0 Anything0
  //-    TVWVar bounded/lower.0 Anything1 }
  private static <T> void verboseWildcard(Optional<T> ovar) {}
}
