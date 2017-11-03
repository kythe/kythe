package pkg;

// Checks that classes are record/class nodes and enums are sum/enumClass nodes

//- @Classes defines/binding N
//- N.node/kind record
//- N.subkind class
public class Classes {

  //- DefaultCtor childof N
  //- DefaultCtor.node/kind function
  //- DefaultCtor typed DefaultCtorType
  //- DefaultCtorType param.0 FnBuiltin
  //- DefaultCtorType param.1 N
  //- !{ DefaultCtorAnchor defines/binding DefaultCtor }

  //- @StaticInner defines/binding SI
  //- SI.node/kind record
  //- SI.subkind class
  //- SI childof N
  private static class StaticInner {
    //- Ctor childof SI
    //- Ctor.node/kind function
    //- Ctor typed CtorType
    //- CtorType param.0 FnBuiltin
    //- CtorType param.1 SI
    //- @StaticInner defines/binding Ctor
    public StaticInner() {}
  }

  //- @Inner defines/binding I
  //- I.node/kind record
  //- I.subkind class
  //- I childof N
  private class Inner {}

  //- @Enum defines/binding E
  //- E.node/kind sum
  //- E.subkind enumClass
  //- E childof N
  private static enum Enum {}

  //- @localFunc defines/binding LF
  private void localFunc() {
    //- @LocalClass defines/binding LC
    //- LC childof LF
    class LocalClass {};
  }


  static {
    //- @LocalClassInStaticInitializer defines/binding LCISI
    //- LCISI childof N
    class LocalClassInStaticInitializer {};
  }
}
