package pkg;

@SuppressWarnings("ClassCanBeStatic")
public class Selectors {
  //- @field defines/binding Field
  //- @String ref String
  String field;

  //- @Optional ref Optional
  //- @maybe defines/binding Param
  public String m(Optional<String> maybe) {
    //- @maybe ref Param
    //- @isPresent ref _IsPresentMethod
    if (maybe.isPresent()) {
      //- @maybe ref Param
      //- @get ref _GetMethod
      //- @field ref Field
      //- @this ref This
      this.field = maybe.get();
    }
    //- @this ref This
    //- @m2 ref M2Method
    //- @toString ref _ToStringMethod
    return this.m2().toString();
  }

  //- @m2 defines/binding M2Method
  private String m2() {
    //- @field ref Field
    //- @this ref This
    return this.field;
  }

  //- @String ref String
  //- @"java.lang" ref _JavaLangPackage
  java.lang.String m3() {
    return null;
  }

  //- @Optional defines/binding Optional
  //- Optional.node/kind interface
  private static interface Optional<T> {
    public T get();
    public boolean isPresent();
  }

  //- @A defines/binding ARecord
  //- AThis typed ARecord
  //- ASuper typed Object
  class A {
    //- @Object ref Object
    Object o;

    //- @str defines/binding AStr
    //- !{ @str defines/binding BStr }
    String str;

    @Override
    public String toString() {
      //- @str ref AStr
      //- @this ref AThis
      //- !{ @this ref BThis }
      return this.str;
    }

    public String toSuperString() {
      //- @super ref ASuper
      //- !{ @super ref BSuper }
      return super.toString();
    }
  }

  @SuppressWarnings("HidingField")
  //- @B defines/binding BRecord
  //- BThis typed BRecord
  //- BSuper typed ARecord
  class B extends A {
    //- @str defines/binding BStr
    //- !{ @str defines/binding AStr }
    String str;

    @Override
    public String toString() {
      //- @str ref BStr
      //- @this ref BThis
      //- !{ @this ref AThis }
      return this.str;
    }

    @Override
    public String toSuperString() {
      //- @str ref AStr
      //- @super ref BSuper
      //- !{ @super ref ASuper }
      return super.str;
    }
  }
}
