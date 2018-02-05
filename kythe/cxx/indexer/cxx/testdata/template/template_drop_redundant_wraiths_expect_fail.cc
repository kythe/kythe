// Checks --experimental_drop_instantiation_independent_data.
template <typename T> class C {
//- @int ref TInt
//- WInt=@int ref/implicit TInt
//- WIField=@ifield defines/binding PTField
//- WInt childof/context CInst
//- WIField childof/context CInst
//- ZInt=@int ref/implicit TInt
//- ZIField=@ifield defines/binding OPTField
//- ZInt childof/context DInst
//- ZIField childof/context DInst
//- CInst specializes TAppCShort
//- DInst specializes TAppCInt
//- TAppCShort param.1 TShort
//- TAppCInt param.1 TInt
  int ifield;
};
//- @short ref TShort
short s;
template class C<short>;
template class C<int>;
