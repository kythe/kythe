// Checks that declarations and definitions of templates are distinguished.

//- @C defines FwdTemplate
template <typename T> class C;

//- @C defines FwdSpec
template <> class C <int>;

//- @C defines Template
//- @C completes/uniquely FwdTemplate
template <typename T> class C { };

//- @C defines Spec
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
