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

//- @Class defines/binding Class=vname("Class#type", _, _, _, _)
//- Class.node/kind record
//- @Class defines/binding ClassValue=vname("Class", _, _, _, _)
//- ClassValue.node/kind function
//- @IFace ref IFace
class Class implements IFace {
    //- @member defines/binding Member
    //- Member.node/kind variable
    //- !{ Member.tag/static _ }
    member: number;

    //- @member defines/binding StaticMember
    //- StaticMember.tag/static _
    //- StaticMember childof Class
    //- !{ @member defines/binding Member }
    static member: number;

    // This ctor declares a new member var named 'otherMember', and also
    // declares an ordinary parameter named 'member' (to ensure we don't get
    // confused about params to the ctor vs true member variables).
    //- @constructor defines/binding ClassCtor=vname("Class.constructor", _, _, _, _)
    //- ClassCtor.node/kind function
    //- ClassCtor.subkind constructor
    //- @otherMember defines/binding OtherMember
    //- OtherMember.node/kind variable
    //- @member defines/binding FakeMember
    //- FakeMember.node/kind variable
    constructor(public otherMember: number, member: string) {
      //- @Class ref ClassValue
      //- @Class ref/call ClassCtor
      new Class(0, 'a');
    }

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
    //- @ifaceMethod defines/binding _ClassIFaceMethod
    ifaceMethod(): void {}
}

//- @Class ref ClassValue
class Subclass extends Class {
    method() {
        //- @member ref Member
        this.member;
    }
}

//- @Class ref ClassValue
let instance = new Class(3, 'a');
//- @otherMember ref OtherMember
instance.otherMember;

// Using Class in type position should still create a link to the class.
//- @Class ref Class
let useAsType: Class = instance;
