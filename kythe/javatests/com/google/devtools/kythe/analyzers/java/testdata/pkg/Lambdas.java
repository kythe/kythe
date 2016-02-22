package pkg;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.function.Function;

//- @Lambdas defines/binding LambdasClass
public class Lambdas {
  //- @fieldName defines/binding Field
  //- Field childof LambdaClass
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
    //- @adderY defines/binding AdderyY
    return adderY
        //- @adderX ref AdderX
        //- @adderY ref AdderY
        -> adderX + adderY;
  }
}
