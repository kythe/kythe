// Checks that we index member variables.
//- @C defines/binding ClassC
//- @f defines/binding FieldF
//- FieldF childof ClassC
//- FieldF typed vname("int#builtin",_,_,_,_)
//- FieldF.node/kind variable
//- FieldF.subkind field
//- FieldF named vname("f:C#n",_,_,_,_)
class C { int f; };
