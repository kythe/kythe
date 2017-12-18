// Checks that implicit ctors and associated detritus are marked as such.
//- @Struct defines/binding RecS
//- RecS.node/kind record
//- !{ @Struct defines/binding OtherS
//-    OtherS.node/kind variable }
//- CtorC.node/kind anchor
//- CtorC defines/binding StructSCtor
//- StructSCtor.node/kind function
//- StructSCtor.complete definition
//- StructSCtor childof RecS
struct Struct {
};

//- @f defines/binding FnF
void f() {
//- Call=@s ref/call StructSCtor
  Struct s;
}

//- Call childof FnF
