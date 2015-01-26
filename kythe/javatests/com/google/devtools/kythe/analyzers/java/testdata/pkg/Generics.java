package pkg;

import java.util.Optional;

// TODO(schroederc): generic class definition tests
public class Generics<T> {

  public static void f() {
    //- @"Generics<String>" ref GType
    //- @gs defines GVar
    //- GVar typed GType
    //- GType.node/kind tapp
    //- GType param.0 Class
    //- GType param.1 Str
    Generics<String> gs =
        //- @"Generics<String>" ref GType
        new Generics<String>();

    //- @"Optional<Generics<String>>" ref OType
    //- OType.node/kind tapp
    //- OType param.0 Optional
    //- OType param.1 GType
    //- @opt defines OVar
    //- OVar typed OType
    Optional<Generics<String>> opt;
  }
}
