// Destructors are indexed.
class C {
  //- @"~C" defines/binding CDtor
  //- CDtor.node/kind function
  //- CDtor.subkind destructor
  ~C() { }
};
