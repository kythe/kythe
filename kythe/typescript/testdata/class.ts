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

//- @IFace ref IFace
interface IExtended extends IFace {}

//- @Class defines/binding Class
//- Class.node/kind record
//- @Class defines/binding ClassCtor
//- ClassCtor.node/kind function
//- ClassCtor.subkind constructor
//- @IFace ref IFace
class Class implements IFace {
    //- @member defines/binding Member
    //- Member.node/kind variable
    //- !{ Member.tag/static _ }
    member: number;

    //- @staticMember defines/binding StaticMember
    //- StaticMember.tag/static _
    static staticMember: number;

    // TODO: ClassCtor should really point at this constructor, not at the
    // top-level class declaration.
    // This ctor declares a new member var named 'otherMember', and also
    // declares an ordinary parameter named 'member' (to ensure we don't get
    // confused about params to the ctor vs true member variables).
    //- @otherMember defines/binding OtherMember
    //- OtherMember.node/kind variable
    //- @member defines/binding FakeMember
    //- FakeMember.node/kind variable
    constructor(public otherMember: number, member: string) {}

    //- @mem defines/binding GetMember
    //- GetMember.node/kind function
    //- GetMember childof Class
    get mem() {
      return this.mem;
    }

    //- @mem defines/binding SetMember
    //- SetMember.node/kind function
    //- SetMember childof Class
    set mem(newMem) {
      //- @member ref Member
      this.member = newMem;
    }

    //- @method defines/binding Method
    //- Method.node/kind function
    //- Method childof Class
    method() {
        //- @member ref Member
        this.member;
        //- @method ref Method
        this.method();
        //- @mem ref GetMember
        this.mem;
        //- @mem ref GetMember
        this.mem = 0;
    }

    // TODO: ensure the method is linked to the interface too.
    //- @ifaceMethod defines/binding _ClassIFaceMethod
    ifaceMethod(): void {}
}

//- @Class ref ClassCtor
class Subclass extends Class {
    method() {
        //- @member ref Member
        this.member;
    }
}

//- @Class ref ClassCtor
let instance = new Class(3, 'a');
//- @otherMember ref OtherMember
instance.otherMember;

// Using Class in type position should still create a link to the class.
//- @Class ref Class
let useAsType: Class = instance;
