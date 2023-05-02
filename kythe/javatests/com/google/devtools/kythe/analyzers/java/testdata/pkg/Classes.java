package pkg;

// Checks that classes are record/class nodes and enums are sum/enumClass nodes

@SuppressWarnings({"unused", "JavaLangClash", "ClassCanBeStatic"})
//- @Classes defines/binding N
//- N.node/kind record
//- N.subkind class
//- DefaultCtrAnchor.loc/start @^class
//- DefaultCtrAnchor.loc/end @^class
public class Classes {

  //- DefaultCtor childof N
  //- DefaultCtor.node/kind function
  //- DefaultCtor typed DefaultCtorType
  //- DefaultCtorType param.0 FnBuiltin
  //- DefaultCtorType param.1 N
  //- DefaultCtrAnchor defines DefaultCtor
  //- !{ _DefaultCtorBindingAnchor defines/binding DefaultCtor }

  //- @Subclass defines/binding SubclassOne
  private static class Subclass extends Classes {
    //- ImplicitSuperCall ref/call/direct DefaultCtor
    //- ImplicitSuperCall.loc/start @^"{}"
    //- ImplicitSuperCall.loc/end @^"{}"
    //- ImplicitSuperCall childof SubclassCtor
    //- @Subclass defines/binding SubclassCtor
    Subclass() {}

    Subclass(String s) {
      //- @"this()" ref/call/direct SubclassCtor
      this();
    }

    Subclass(int i) {
      //- @"super()" ref/call/direct DefaultCtor
      super();
    }
  }

  //- @Subclass2 defines/binding SubclassTwo
  //- ImplicitSubclassTwoCtorDef.node/kind anchor
  //- ImplicitSubclassTwoCtorDef.subkind implicit
  //- ImplicitSubclassTwoCtorDef defines ImplicitSubclassTwoCtor
  //- ImplicitSubclassTwoCtorDef.loc/start @^#0class
  //- ImplicitSubclassTwoCtorDef.loc/end @^#0class
  //- ExtraImplicitSuperCall.node/kind anchor
  //- ExtraImplicitSuperCall.subkind implicit
  //- ExtraImplicitSuperCall.loc/start @^#0class
  //- ExtraImplicitSuperCall.loc/end @^#0class
  private static class Subclass2 extends Classes {
    //- ImplicitSubclassTwoCtor.node/kind function
    //- ImplicitSubclassTwoCtor.subkind constructor
    //- ImplicitSubclassTwoCtor childof SubclassTwo
    //- ExtraImplicitSuperCall ref/call/direct DefaultCtor
    //- ExtraImplicitSuperCall childof ImplicitSubclassTwoCtor
  }

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
    class LocalClass {}
  }

  //- ClassInit.node/kind function
  //- ClassInit childof N
  // !{ ClassInit.subkind "constructor" }

  static {
    //- @LocalClassInStaticInitializer defines/binding LCISI
    //- LCISI childof ClassInit
    class LocalClassInStaticInitializer {}
  }
}

//- SubclassOne.tag/static _
//- SubclassTwo.tag/static _
//- SI.tag/static _
//- E.tag/static _
//- !{I.tag/static _}
