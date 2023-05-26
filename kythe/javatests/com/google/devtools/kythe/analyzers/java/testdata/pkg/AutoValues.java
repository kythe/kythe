package com.google.devtools.kythe.analyzers.java.testdata.pkg;

import com.google.auto.value.AutoValue;

//- File=vname("", Corpus, Root, Path, "").node/kind file
//- GeneratedFile=vname("", GenCorpus, GenRoot, GenPath, "").node/kind file
//- File generates GeneratedFile

//- GeneratedDef=vname(_, GenCorpus, GenRoot, GenPath, _).node/kind anchor
//- GeneratedDef defines/binding AutoStringPair

@SuppressWarnings("unused")
//- @AutoValues=vname(_, Corpus, Root, Path, _).node/kind anchor
public final class AutoValues {
  private AutoValues() {}

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
      //- FirstSetter.semantic/generated set
      public abstract Builder setFirst(String first);

      //- FirstSetterImpl overrides FirstSetter
      //- FirstSetterImpl property/writes FirstProp
      //- FirstSetter generates FirstSetterImpl

      //- @setSecond defines/binding SecondSetter
      //- SecondSetter property/writes SecondProp
      //- SecondSetter.semantic/generated set
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
      //- SetString.semantic/generated set
      public abstract Builder setString(String s);

      //- @setBool defines/binding SetBool
      //- SetBool property/writes BoolProp
      //- SetBool.semantic/generated set
      public abstract Builder setBool(boolean b);

      public abstract WithPrefixes build();
    }
  }

  @AutoValue
  public abstract static class AsInterface {
    //- @getObject defines/binding GetObject
    //- GetObject property/reads ObjectProp
    public abstract Object getObject();

    //- GetObject generates GetObjectImpl
    //- GetObjectImpl overrides GetObject
    //- GetObjectImpl property/reads ObjectProp

    @AutoValue.Builder
    interface Builder {
      //- @object defines/binding SetObject
      //- SetObject property/writes ObjectProp
      //- SetObject.semantic/generated set
      public Builder object(Object o);

      //- SetObject generates SetObjectImpl
      //- SetObjectImpl overrides SetObject
      //- SetObjectImpl property/writes ObjectProp

      public AsInterface build();
    }
  }
}
