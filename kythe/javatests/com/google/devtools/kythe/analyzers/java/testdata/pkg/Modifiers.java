package pkg;

@SuppressWarnings("unused")
public abstract class Modifiers {

  //- @privMember defines/binding PrivateMember
  private int privMember;
  //- @volMember defines/binding VolatileMember
  private volatile int volMember;

  //- @method defines/binding Method
  int method() {
    return 0;
  }

  //- @absmethod defines/binding AbstractMethod
  abstract int absmethod();

  interface Intf {
    //- @func defines/binding IMethod
    int func();

    //- @defFunc defines/binding DefaultMethod
    default int defFunc() {
      return 0;
    }
  }
}

//- !{ PrivateMember.tag/volatile _ }
//- VolatileMember.tag/volatile _

//- !{ IMethod.tag/default _ }
//- DefaultMethod.tag/default _

//- !{ Method.tag/abstract _ }
//- AbstractMethod.tag/abstract _
