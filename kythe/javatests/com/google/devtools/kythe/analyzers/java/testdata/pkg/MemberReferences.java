package pkg;

import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.function.Supplier;

//- @Referent defines/binding ReferentClass
class Referent {
  //- @Referent defines/binding NullaryConstructor
  //- NullaryConstructor childof ReferentClass
  public Referent() {}

  //- @Referent defines/binding IntegerConstructor
  //- IntConstructor childof ReferentClass
  public Referent(Integer x) {}
  
  //- @Referent defines/binding ObjectConstructor
  //- ObjectConstructor childof ReferentClass
  public Referent(Object x) {}
  
  //- @Referent defines/binding BinaryConstructor
  //- BinaryConstructor childof ReferentClass
  public Referent(Object x, Object y) {}

  //- @func defines/binding StaticIterableMethod
  //- StaticIterableMethod childof ReferentClass
  public static <T> Iterable<T> func(Iterable<T> x) {
    return x;
  }

  //- @func defines/binding StaticIntMethod
  //- StaticIntMethod childof ReferentClass
  public static Referent func(int x) {
    return null;
  }

  //- @func defines/binding IdentityMethod
  //- IdentityMethod childof ReferentClass
  public Referent func() {
    return this;
  }

  //- @func defines/binding NullaryMethod
  //- AlternativeMethod childof ReferentClass
  public Referent func(Referent x) {
    return x;
  }
}

public class MemberReferences {
  public Function<Referent, Referent> getIdFunction() {
    //- @Referent ref ReferentClass
    //- @func ref IdentityMethod
    return Referent::func;
  }

  public Function<Integer, Referent> getStaticIntFunction() {
    //- @Referent ref ReferentClass
    //- @func ref StaticIntMethod
    return Referent::func;
  }

  public Function<Iterable<?>, Iterable<?>> getStaticIterableFunction() {
    //- @Referent ref ReferentClass
    //- @func ref StaticIterableMethod
    return Referent::func;
  }

  public BiFunction<Referent, Referent, Referent> getBiFunction() {
    //- @Referent ref ReferentClass
    //- @func ref AlternativeMethod
    return Referent::func;
  }

  public Supplier<Referent> getIdThunk() {
    //- @Referent ref ReferentClass
    //- @func ref IdentityMethod
    //- !{ @new.node/kind anchor }
    return new Referent()::func;
  }

  public Function<Referent, Referent> getIdPartialApplication() {
    //- @Referent ref ReferentClass
    //- @func ref AlternativeMethod
    //- !{ @new.node/kind anchor }
    return new Referent()::func;
  }

  public Function<Object, Referent> getConstructorObjectFunction() {
    //- @Referent ref ReferentClass
    //- @new ref ObjectConstructor
    return Referent::new;
  }

  public Function<Integer, Referent> getConstructorIntegerFunction() {
    //- @Referent ref ReferentClass
    //- @new ref IntegerConstructor
    return Referent::new;
  }

  public BiFunction<Object, Object, Referent> getConstructorBiFunction() {
    //- @Referent ref ReferentClass
    //- @new ref BinaryConstructor
    return Referent::new;
  }

  public Supplier<Referent> getConstructorThunk() {
    //- @Referent ref ReferentClass
    //- @new ref NullaryConstructor
    return Referent::new;
  }
}
