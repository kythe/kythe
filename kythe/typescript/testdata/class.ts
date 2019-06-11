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
//- @IFace ref IFace
class Class implements IFace {
    //- @member defines/binding Member
    //- Member.node/kind variable
    //- Member childof Class
    //- !{ Member.tag/static _ }
    member: number;

    //- @staticMember defines/binding StaticMember
    //- StaticMember.tag/static _
    //- StaticMember childof Class
    static staticMember: number;

    // This ctor declares a new member var named 'otherMember', and also
    // declares an ordinary parameter named 'member' (to ensure we don't get
    // confused about params to the ctor vs true member variables).
    //- @constructor defines/binding ClassCtor
    //- ClassCtor.node/kind function
    //- ClassCtor.subkind constructor
    //- @otherMember defines/binding OtherMember
    //- OtherMember.node/kind variable
    //- OtherMember childof Class
    //- @member defines/binding FakeMember
    //- FakeMember.node/kind variable
    //- !{ FakeMember childof Class }
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
    //- @ifaceMethod defines/binding _ClassIFaceMethod
    ifaceMethod(): void {}
}

// A class without a constructor binds the constructor to its declaration.
//- @Class ref ClassCtor
//- @Subclass defines/binding SubClassCtor
//- SubClassCtor.node/kind function
//- SubClassCtor.subkind constructor
class Subclass extends Class {
    method() {
        //- @member ref Member
        this.member;

        //- @Class ref ClassCtor
        new Class(0, '');

        //- @Class ref Class
        type CC = Class;
    }
}

//- @Class ref ClassCtor
let instance = new Class(3, 'a');
//- @otherMember ref OtherMember
instance.otherMember;

// Using Class in type position should still create a link to the class.
//- @Class ref Class
let useAsType: Class = instance;
