package pkg;

import java.util.function.Function;

@SuppressWarnings({"unused", "JavaLangClash", "UnnecessaryLambda"})
//- @Lambdas defines/binding LambdasClass
public class Lambdas {
  //- @Function ref FuncAbs
  //- Func childof FuncAbs
  //- @"x -> x" defines IdentLambda
  //- IdentLambda.node/kind function
  //- IdentLambda extends Func
  private static final Function<Object, Object> IDENTITY = x -> x;

  //- @fieldName defines/binding Field
  //- Field childof LambdasClass
  Object fieldName;

  //- @num defines/binding NumParameter1
  public int callLambda(int num) {
    //- @getAdder ref GetAdder
    //- @x defines/binding XLambdaParameter
    return getAdder(5).compose((Integer x)
        //- @x ref XLambdaParameter
        //- @num ref NumParameter1
        -> x*2).apply(num);
  }

  //- @num defines/binding NumParameter2
  public int callWhileShadowing(int num) {
    //- @getAdder ref GetAdder
    //- @fieldName defines/binding ShadowingParameter
    return getAdder(5).compose((Integer fieldName)
        //- @fieldName ref ShadowingParameter
        //- @num ref NumParameter2
        -> fieldName*2).apply(num);
  }

  //- @getAdder defines/binding GetAdder
  //- @adderX defines/binding AdderX
  public Function<Integer, Integer> getAdder(final int adderX) {
    //- @adderY defines/binding AdderY
    return adderY
        //- @adderX ref AdderX
        //- @adderY ref AdderY
        -> adderX + adderY;
  }

  //- @String defines/binding StrCopy
  private static final class String { // purposefully clash with builtin class
    public static String create() { return new String(); }
  }

  //- @#0String ref StrBuiltin
  //- @#1String ref StrCopy
  public Function<java.lang.String, String> checkParameterAnchors() {
    //- @String ref StrCopy
    //- !{ @String ref StrBuiltin }
    return s -> String.create();
  }
}
