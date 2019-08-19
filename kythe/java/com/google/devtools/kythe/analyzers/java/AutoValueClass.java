package com.google.devtools.kythe.analyzers.java;

import com.google.auto.value.AutoValue;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Streams;
import com.sun.tools.javac.code.Symbol;
import java.util.Optional;
import java.util.stream.Stream;

@AutoValue
/* Record of resolved {@link Symbol}s for an {@link AutoValue} and its generated class. */
abstract class AutoValueClass {
  @AutoValue
  abstract static class Property {
    abstract String name();

    abstract GeneratedSymbol getter();

    abstract Optional<GeneratedSymbol> setter();

    static Builder builder() {
      return new AutoValue_AutoValueClass_Property.Builder();
    }

    Stream<GeneratedSymbol> stream() {
      return Streams.concat(Stream.of(getter()), Streams.stream(setter()));
    }

    @AutoValue.Builder
    abstract static class Builder {
      abstract Builder setName(String s);

      abstract Builder setGetter(GeneratedSymbol s);

      abstract Builder setSetter(GeneratedSymbol s);

      abstract Property build();
    }
  }

  abstract GeneratedSymbol symbol();

  abstract Optional<GeneratedSymbol> builderSymbol();

  abstract ImmutableSet<Property> properties();

  Stream<GeneratedSymbol> stream() {
    return Streams.concat(
        Stream.of(symbol()),
        Streams.stream(builderSymbol()),
        properties().stream().flatMap(Property::stream));
  }

  static Builder builder() {
    return new AutoValue_AutoValueClass.Builder();
  }

  @AutoValue.Builder
  abstract static class Builder {
    abstract Builder setSymbol(GeneratedSymbol s);

    abstract Builder setBuilderSymbol(GeneratedSymbol s);

    abstract ImmutableSet.Builder<Property> propertiesBuilder();

    Builder addProperty(Property p) {
      propertiesBuilder().add(p);
      return this;
    }

    abstract AutoValueClass build();
  }

  @AutoValue
  abstract static class GeneratedSymbol {
    abstract Symbol abstractSym();

    abstract Symbol generatedSym();

    static Builder builder() {
      return new AutoValue_AutoValueClass_GeneratedSymbol.Builder();
    }

    Stream<Symbol> stream() {
      return Stream.of(abstractSym(), generatedSym());
    }

    @AutoValue.Builder
    abstract static class Builder {
      abstract Builder setAbstractSym(Symbol s);

      abstract Builder setGeneratedSym(Symbol s);

      abstract GeneratedSymbol build();
    }
  }
}
