package com.google.devtools.kythe.analyzers.java.testdata.pkg;

import com.google.auto.value.AutoValue;

public final class AutoValues {
  @AutoValue
  //- @StringPair defines/binding StringPair
  public abstract static class StringPair {
    abstract String first();

    abstract String second();

    public static StringPair create(String first, String second) {
      //- @AutoValue_AutoValues_StringPair ref AutoStringPair
      //- AutoStringPair extends StringPair
      return new AutoValue_AutoValues_StringPair(first, second);
    }
  }
}
