struct S {
  //- @S defines/binding SCtorDefault
  S();
  //- @#0S defines/binding SCtorMove
  S(S&&);
};
// A simple alias to make goals on subsequent functions
// easier to read.
using Ignore = S;

//- @S ref SCtorDefault
//- @"S{}" ref/call SCtorDefault
//- @"S{}" ref/call SCtorMove
//- !{ @S ref SCtorMove }
Ignore f() { return S{}; }
//- @#1g defines/binding GFunc
//- @#0S ref SCtorMove  // This is an explicit call.
//- @#1S ref SCtorDefault
//- @"S{}" ref/call SCtorDefault
//- GOuterS=@"S{S{}}" ref/call SCtorMove
//- GOuterS.loc/start GOuterSStart
//- GZOuterS ref SCtorMove
//- GZOuterS.loc/start GOuterSStart
Ignore g() { return S{S{}}; }
//- @"g()" ref/call SCtorMove
//- @"g()" ref/call GFunc
//- !{ @#1g ref SCtorMove }
Ignore h() { return g(); }
//- @S ref SCtorDefault
//- @S ref/call SCtorDefault
Ignore* i() { return new S; }
//- @S ref SCtorDefault
//- @"S{}" ref/call SCtorDefault
Ignore* j() { return new S{}; }
//- @#0S ref SCtorMove  // This is an explicit call.
//- @#1S ref SCtorDefault
//- OuterS=@"S{S{}}" ref/call SCtorMove
//- @"S{}" ref/call SCtorDefault
//- KZOuterS ref SCtorMove
//- OuterS.loc/start OuterSStart
//- KZOuterS.loc/start OuterSStart
Ignore* k() { return new S{S{}}; }
