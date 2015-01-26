// Checks that record nodes underneath abstractions are given distinct names.
// (This test's verifier load is mainly in the well-formedness checks.)

//- @C defines TemplateC
//- TemplateC.node/kind abs
//- TemplaceCBody childof TemplateC
//- TemplaceCBody.node/kind record
template <typename T, typename S> class C { };

//- @C defines PartialSpecializationC
//- PartialSpecializationC.node/kind abs
template <typename U> class C<int, U> { };

//- @C defines TotalSpecializationC
//- TotalSpecializationC.node/kind record
template <> class C<int, float> { };
