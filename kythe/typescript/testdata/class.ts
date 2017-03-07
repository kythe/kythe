export {}

//- @IFace defines/binding IFace
//- IFace.node/kind interface
interface IFace {
  //- @ifaceMethod defines/binding IFaceMethod
  //- IFaceMethod.node/kind function
  //- IFaceMethod.complete incomplete
  //- IFaceMethod childof IFace
  ifaceMethod(): void;
}

//- @Class defines/binding Class
//- Class.node/kind record
class Class implements IFace {
  //- @member defines/binding Member
  //- Member.node/kind variable
  member: number;

  //- @method defines/binding Method
  //- Method.node/kind function
  //- Method childof Class
  method() {
    //- @member ref Member
    this.member;
    //- @method ref Method
    this.method();
  }

  // TODO: ensure the method is linked to the interface too.
  //- @ifaceMethod defines/binding ClassIFaceMethod
  ifaceMethod(): void {}
}

// TODO: subclass, extends/implements, generics, etc.
