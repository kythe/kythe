export {}

//- @Base defines/binding Base
//- Base.node/kind record
class Base {
  //- @member defines/binding Member
  //- Member.node/kind variable
  member: number;

  //- @method defines/binding Method
  //- Method.node/kind function
  method() {
    //- @member ref Member
    this.member;
    //- @method ref Method
    this.method();
  }
}

// TODO: subclass, extends/implements, generics, etc.
