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
    //- IFaceMember.subkind field
    //- IFaceMember childof IFace
    member: number;
}

//- @IFace ref IFace
//- @IExtended defines/binding IExtended
//- IExtended extends IFace
interface IExtended extends IFace {
    //- @ifaceMethod defines/binding ExtendedIFaceMethod
    //- ExtendedIFaceMethod overrides IFaceMethod
    ifaceMethod(): void;
}

//- @Class defines/binding Class=vname("Class#type", _, _, _, _)
//- Class.node/kind record
//- @IFace ref IFace
//- Class extends IFace
class Class implements IFace {
    //- @member defines/binding Member
    //- Member.node/kind variable
    //- !{ Member.tag/static _ }
    //- Member.subkind field
    //- Member childof Class
    member: number;

    //- @member defines/binding StaticMember
    //- StaticMember.tag/static _
    //- !{ @member defines/binding Member }
    //- StaticMember childof Class
    static member: number;

    // This ctor declares a new member var named 'otherMember', and also
    // declares an ordinary parameter named 'member' (to ensure we don't get
    // confused about params to the ctor vs true member variables).
    //- @constructor defines/binding ClassCtor=vname("Class", _, _, _, _)
    //- ClassCtor.node/kind function
    //- ClassCtor.subkind constructor
    //- ClassCtor childof Class
    //- @otherMember defines/binding OtherMember
    //- OtherMember.node/kind variable
    //- @member defines/binding FakeMember
    //- FakeMember.node/kind variable
    constructor(public otherMember: number, member: string) {
      //- @Class ref ClassCtor
      //- @"new Class(0, 'a')" ref/call ClassCtor
      new Class(0, 'a');
    }

    //- @method defines/binding Method
    //- Method.node/kind function
    //- Method childof Class
    method() {
        //- @this ref Class
        //- @member ref Member
        this.member;
        //- @this ref Class
        //- @method ref Method
        this.method();
    }

    //- @ifaceMethod defines/binding ClassIFaceMethod
    //- ClassIFaceMethod overrides IFaceMethod
    ifaceMethod(): void {}
}


//- @Subclass defines/binding Subclass=vname("Subclass#type", _, _, _, _)
//- Subclass.node/kind record
//- @Subclass defines/binding SubclassCtor=vname("Subclass", _, _, _, _)
//- SubclassCtor.node/kind function
//- @Class ref Class
class Subclass extends Class {
    //- @method defines/binding OverridenMethod
    //- OverridenMethod overrides Method
    override method() {
        //- @member ref Member
        this.member;
    }
}

class SubSubclass extends Subclass implements IExtended {
    //- @ifaceMethod defines/binding OverridenIFaceMethod
    //- OverridenIFaceMethod overrides ClassIFaceMethod
    //- OverridenIFaceMethod overrides ExtendedIFaceMethod
    //- !{ OverridenIFaceMethod overrides IFaceMethod }
    override ifaceMethod() {}
}

//- @Class ref ClassCtor
//- @Class ref/id Class
let instance = new Class(3, 'a');
//- @otherMember ref OtherMember
instance.otherMember;

// Using Class in type position should still create a link to the class.
//- @Class ref Class
let useAsType: Class = instance;

//- @Class ref Class
Class;
