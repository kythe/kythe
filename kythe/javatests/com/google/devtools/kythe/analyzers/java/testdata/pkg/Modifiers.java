package pkg;

@SuppressWarnings("unused")
public class Modifiers {

  //- @privMember defines/binding PrivateMember
  private int privMember;
  //- @volMember defines/binding VolatileMember
  private volatile int volMember;

  interface Intf {
    //- @func defines/binding Method
    int func();

    //- @defFunc defines/binding DefaultMethod
    default int defFunc() { return 0; }
  }
}

//- !{ PrivateMember.tag/volatile _ }
//- VolatileMember.tag/volatile _

//- !{ Method.tag/default _ }
//- DefaultMethod.tag/default _
