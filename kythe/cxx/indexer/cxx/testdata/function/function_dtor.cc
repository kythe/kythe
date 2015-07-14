// Destructors are indexed.
class C {
  //- @"~C" defines CDtor
  //- CDtor callableas CDtorC
  //- CDtor named vname("~C:C#n",_,_,_,_)
  //- CDtor.node/kind function
  //- CDtor.subkind destructor
  ~C() { }
};
