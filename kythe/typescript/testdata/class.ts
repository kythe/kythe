export {}

//- @IFace defines/binding IFace
//- IFace.node/kind interface
interface IFace {
  //- @ifaceMethod defines/binding IFaceMethod
  //- IFaceMethod.node/kind function
  //- IFaceMethod.complete incomplete
  //- IFaceMethod childof IFace
  ifaceMethod(): void;

  //- @member defines/binding IFaceMember
  //- IFaceMember.node/kind variable
  member: number;
}

//- @Class defines/binding Class
//- Class.node/kind record
//- @IFace ref Iface
class Class implements IFace {
  //- @member defines/binding Member
  //- Member.node/kind variable
  member: number;

  // This ctor declares a new member var named 'otherMember', and also
  // declares an ordinary parameter named 'member' (to ensure we don't get
  // confused about params to the ctor vs true member variables).
  //- @otherMember defines/binding OtherMember
  //- OtherMember.node/kind variable
  //- @member defines/binding FakeMember
  //- FakeMember.node/kind variable
  constructor(public otherMember: number, member: string) {}

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

let instance = new Class(3, 'a');
//- @otherMember ref OtherMember
instance.otherMember;

// TODO: subclass, extends/implements, generics, etc.
