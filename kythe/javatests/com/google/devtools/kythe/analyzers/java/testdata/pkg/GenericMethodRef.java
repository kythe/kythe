package pkg;

@SuppressWarnings("unused")
//- @GenericMethodRef defines/binding GClass
public final class GenericMethodRef {
  //- @Optional defines/binding OClass
  public static final class Optional<T> {
    //- @Optional defines/binding OptionalConstructor
    Optional() {}
  }

  // TODO(#1501): wildcard tests currently fail
  //- @Optional ref OClass
  //- @wildcard defines/binding WildcardFnDecl
  //- @ovar defines/binding WildcardParam1
  //- WildcardFnDecl param.0 WildcardParam1
  private static void wildcard(Optional<?> ovar) {}

  //- @verboseWildcard defines/binding VerboseWildcardFnDecl
  //- @ovar defines/binding VWParam1
  //- VerboseWildcardFnDecl.node/kind function
  //- VerboseWildcardFnDecl param.0 VWParam1
  private static <T> void verboseWildcard(Optional<T> ovar) {}

  private static void caller() {
    // - @wildcard ref WildcardFnDecl
    // - @"wildcard(null)" ref/call WildcardFnDecl
    wildcard(null);

    // - @verboseWildcard ref VerboseWildcardFnDecl
    // - @"verboseWildcard(null)" ref/call VerboseWildcardFnDecl
    verboseWildcard(null);
  }

  //- @T defines/binding AbsT
  private static <T> void constructor() {
    //- @Optional ref OptionalConstructor
    //- @Optional ref/id OClass
    //- @GenericMethodRef ref GClass
    //- @"new Optional<GenericMethodRef>()" ref/call/direct OptionalConstructor
    Object o = new Optional<GenericMethodRef>();

    //- @Optional ref OptionalConstructor
    //- @Optional ref/id OClass
    //- @"new Optional<T>()" ref/call/direct OptionalConstructor
    //- @T ref AbsT
    Object o2 = new Optional<T>();
  }

}
