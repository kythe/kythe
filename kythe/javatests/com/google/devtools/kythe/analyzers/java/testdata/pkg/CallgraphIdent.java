package pkg;

@SuppressWarnings({"unused", "MultiVariableDeclaration"})
//- @CallgraphIdent defines/binding Class
//- ClassInitDef.loc/start @^"CallgraphIdent"
//- ClassInitDef.loc/end @^"CallgraphIdent"
//- ClassInitDef defines ClassInit
public class CallgraphIdent {

  // Implicit static class initializer
  //- ClassInit.node/kind function
  //- ClassInit childof Class
  //- !{ ClassInit.subkind "constructor" }

  //- @g=NonStaticGCall ref/call G
  //- NonStaticGCall childof ECtor
  //- NonStaticGCall childof SCtor
  //- !{ NonStaticGCall childof Class }
  //- !{ NonStaticGCall childof ClassInit }
  final int zero = g();

  //- @g=StaticGCall ref/call G
  //- StaticGCall childof ClassInit
  //- !{ StaticGCall childof ECtor }
  //- !{ StaticGCall childof SCtor }
  static final int ZERO_TWO = g();

  static {
    //- @g=StaticBlockGCall ref/call G
    //- StaticBlockGCall childof ClassInit
    //- !{ StaticBlockGCall childof ECtor }
    //- !{ StaticBlockGCall childof SCtor }
    int zero = g();

    {
      //- @g=NestedStaticBlockGCall ref/call G
      //- NestedStaticBlockGCall childof ClassInit
      //- NestedStaticBlockGCall childof ClassInit
      g();
    }

    //- @h=HCall ref/call H
    //- HCall childof ClassInit
    //- @n=NestedCall ref/call N
    //- NestedCall childof ClassInit
    h().n();
  }

  {
    //- @g=BlockGCall ref/call G
    //- BlockGCall childof ECtor
    //- BlockGCall childof SCtor
    int zero = g();
  }

  //- @CallgraphIdent ref/call/direct ECtor
  //- @CallgraphIdent ref ECtor
  //- CtorCall childof ECtor
  //- CtorCall childof SCtor
  //- @CallgraphIdent ref/id Class
  final Object instance = new CallgraphIdent();

  //- @CallgraphIdent defines/binding ECtor
  //- ECtor.node/kind function
  //- ECtor.subkind constructor
  public CallgraphIdent() {}

  //- @CallgraphIdent defines/binding SCtor
  //- SCtor.node/kind function
  //- SCtor.subkind constructor
  public CallgraphIdent(String s) {}

  //- @f defines/binding F
  //- F.node/kind function
  //- F typed _FType
  static void f(int n) {
    //- @CallgraphIdent ref ECtor
    //- @CallgraphIdent ref/id Class
    //- @CallgraphIdent=ECtorCall ref/call/direct ECtor
    //- ECtorCall childof F
    Object cg = new CallgraphIdent();
    //
    //- @CallgraphIdent ref SCtor
    //- @CallgraphIdent ref/id Class
    //- @CallgraphIdent=SCtorCall ref/call/direct SCtor
    //- SCtorCall childof F
    cg = new CallgraphIdent(null);
  }

  //- @g defines/binding G
  //- G.node/kind function
  static int g() {
    //- @f=CallAnchor ref/call F
    //- CallAnchor childof  G
    f(4);

    return 0;
  }

  //- @h defines/binding H
  static Nested h() {
    return new Nested();
  }

  //- @Nested defines/binding NestedClass
  static class Nested {
    //- NestedInit.node/kind function
    //- NestedInit childof NestedClass
    //- !{ NestedInit.subkind "constructor" }

    //- ImplicitConstructor.node/kind function
    //- ImplicitConstructor.subkind "constructor"
    //- ImplicitConstructor childof NestedClass

    //- @g=NestedClassCall childof ImplicitConstructor
    final int someInt = g();

    static {
      //- @g=NestedClassStaticCall childof NestedInit
      g();
    }

    //- @n defines/binding N
    void n() {}

    //- !{ NestedClassCall childof NestedInit
    //-    NestedClassStaticCall childof ImplicitConstructor }
  }
}
