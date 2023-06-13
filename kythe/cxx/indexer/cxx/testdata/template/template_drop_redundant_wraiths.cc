// Checks --experimental_drop_instantiation_independent_data.
template <typename T> class C {
//- @int ref TInt
//- WInt=@int ref TInt
//- WIField=@ifield defines/binding PTField
//- WIField childof/context CInst
  int ifield;
};
template class C<short>;
