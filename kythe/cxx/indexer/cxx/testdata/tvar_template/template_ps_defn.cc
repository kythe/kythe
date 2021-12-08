// Checks that declarations and definitions of templates are distinguished.

//- @C defines/binding FwdTemplate
template <typename T> class C;

//- @C defines/binding FwdSpec
template <> class C <int>;

//- @C defines/binding Template
//- @C completes/uniquely FwdTemplate
template <typename T> class C { };

//- @C defines/binding Spec
//- @C completes/uniquely FwdSpec
template <> class C <int> { };

//- FwdTemplate.node/kind abs
//- FwdSpec.node/kind record
//- FwdSpec.complete incomplete
//- Fwd.node/kind abs
//- Spec.node/kind record
//- Spec.complete definition
//- Spec specializes TAppCInt
//- FwdSpec specializes TAppCInt
