package pkg;

@SuppressWarnings("unused")
public final class GenericMethod {
  public static final class Optional<T> {}

  // - @Optional ref OptionalClass
  // - @wildcard defines/binding WildcardFnDecl
  // - @ovar defines/binding WildcardParam1
  // - WildcardFnDecl.node/kind function
  // - WildcardFnDecl param.0 WildcardParam1
  // - WildcardParam1 typed WildcardParam1Type
  // - WildcardParam1Type.node/kind tapp
  // - WildcardParam1Type param.0 OptionalClass
  // - WildcardParam1Type param.1 Wildcard0
  // - Wildcard0.node/kind tvar
  // - !{ Wildcard0 bounded/upper Anything0
  // -    Wildcard0 bounded/lower Anything1 }
  private static void wildcard(Optional<?> ovar) {}

  // - @#0T defines/binding TVWVar
  // - @verboseWildcard defines/binding VerboseWildcardFnDecl
  // - @ovar defines/binding VWParam1
  // - TVWVar.node/kind tvar
  // - VerboseWildcardFnDecl.node/kind function
  // - VerboseWildcardFnDecl tparam.0 TVWVar
  // - VerboseWildcardFnDecl param.0 VWParam1
  // - VWParam1 typed VWParam1Type
  // - VWParam1Type.node/kind tapp
  // - VWParam1Type param.0 OptionalClass
  // - VWParam1Type param.1 TVWVar
  // - !{ TVWVar bounded/upper.0 Anything0
  // -    TVWVar bounded/lower.0 Anything1 }
  private static <T> void verboseWildcard(Optional<T> ovar) {}
}
