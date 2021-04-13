package pkg;

@SuppressWarnings("unused")
//- @Modifiers defines/binding AbstractClass
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

  //- @Clazz defines/binding Class
  class Clazz { }

  //- @Intf defines/binding Interface
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

//- !{ Class.tag/abstract _ }
//- AbstractClass.tag/abstract _
//- !{ Interface.tag/abstract _ }
