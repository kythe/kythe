package com.google.devtools.kythe.analyzers.java.testdata.pkg;

import com.google.auto.value.AutoValue;

public final class AutoValues {
  @AutoValue
  //- @StringPair defines/binding StringPair
  public abstract static class StringPair {
    //- FirstProp.node/kind property
    //- SecondProp.node/kind property

    //- @first defines/binding FirstGetter
    //- FirstGetter property/reads FirstProp
    abstract String first();

    //- FirstGetterImpl overrides FirstGetter
    //- FirstGetterImpl property/reads FirstProp
    //- FirstGetter generates FirstGetterImpl

    //- @second defines/binding SecondGetter
    //- SecondGetter property/reads SecondProp
    abstract String second();

    //- @AutoValue_AutoValues_StringPair ref AutoStringPair
    //- AutoStringPair extends StringPair
    //- StringPair generates AutoStringPair
    private static final AutoValue_AutoValues_StringPair IMPL = null;

    @AutoValue.Builder
    //- @Builder defines/binding StringPairBuilder
    public abstract static class Builder {
      //- @setFirst defines/binding FirstSetter
      //- FirstSetter property/writes FirstProp
      public abstract Builder setFirst(String first);

      //- FirstSetterImpl overrides FirstSetter
      //- FirstSetterImpl property/writes FirstProp
      //- FirstSetter generates FirstSetterImpl

      //- @setSecond defines/binding SecondSetter
      //- SecondSetter property/writes SecondProp
      public abstract Builder setSecond(String second);

      public abstract StringPair build();

      //- @Builder ref AutoStringPairBuilder
      //- AutoStringPairBuilder extends StringPairBuilder
      //- StringPairBuilder generates AutoStringPairBuilder
      private static final AutoValue_AutoValues_StringPair.Builder IMPL = null;
    }
  }

  @AutoValue
  public abstract static class WithPrefixes {
    //- @getString defines/binding GetString
    //- GetString property/reads StringProp
    public abstract String getString();

    //- @isBool defines/binding GetBool
    //- GetBool property/reads BoolProp
    public abstract boolean isBool();

    @AutoValue.Builder
    public abstract static class Builder {
      //- @setString defines/binding SetString
      //- SetString property/writes StringProp
      public abstract Builder setString(String s);

      //- @setBool defines/binding SetBool
      //- SetBool property/writes BoolProp
      public abstract Builder setBool(boolean b);

      public abstract WithPrefixes build();
    }
  }
}
