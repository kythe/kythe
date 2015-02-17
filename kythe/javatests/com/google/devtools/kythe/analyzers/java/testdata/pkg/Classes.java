package pkg;

// Checks that classes are record/class nodes and enums are sum/enumClass nodes

//- @Deprecated ref Deprecated
@Deprecated
//- @Classes defines N
//- N.node/kind record
//- N.subkind class
public class Classes {

  //- DefaultCtor childof N
  //- DefaultCtor.node/kind function
  //- DefaultCtor typed DefaultCtorType
  //- DefaultCtorType param.0 FnBuiltin
  //- DefaultCtorType param.1 N
  // TODO(schroederc): Don't emit an anchor for the default constructor
  // !{ DefaultCtorAnchor defines DefaultCtor }

  //- @StaticInner defines SI
  //- SI.node/kind record
  //- SI.subkind class
  //- SI childof N
  private static class StaticInner {}

  //- @Inner defines I
  //- I.node/kind record
  //- I.subkind class
  //- I childof N
  private class Inner {}

  //- @Enum defines E
  //- E.node/kind sum
  //- E.subkind enumClass
  //- E childof N
  private static enum Enum {}

  //- @Deprecated ref Deprecated
  //- @Override ref Override
  @Override @Deprecated
  public String toString() {
    return "Classes";
  }
}
