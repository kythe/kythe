export {};

// This file tests 'class expression', which is use of the 'class' keyword
// in some expression positions.

// JS allows 'extends' of arbitrary expressions.
class Extends extends (
class Base {
  //- @member defines/binding Member
  member: string;
}) {
  method() {
    //- @member ref Member
    this.member;
  }
}

const unnamed = class {
  //- @member2 defines/binding Member2
  member2: string;
  method() {
    //- @member2 ref Member2
    this.member2;
  }
};
