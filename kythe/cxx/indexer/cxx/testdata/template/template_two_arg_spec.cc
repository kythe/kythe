// Checks that specializations of record nodes underneath abstractions are
// given distinct names.
// (This test's verifier load is mainly in the well-formedness checks.)

//- @C defines/binding TemplateCBody
//- TemplaceCBody.node/kind record
template <typename T, typename S> class C { };

//- @C defines/binding PartialSpecializationC
//- PartialSpecializationC.node/kind record
template <typename U> class C<int, U> { };

//- @C defines/binding TotalSpecializationC
//- TotalSpecializationC.node/kind record
template <> class C<int, float> { };
