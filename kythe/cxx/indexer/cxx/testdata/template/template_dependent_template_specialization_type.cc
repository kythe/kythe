// We index dependent template specialization types.
//- @U defines/binding TvarU
template <typename T, typename U>
//- @A defines/binding AliasA
using A = T::template S<U>;
//- AliasA aliases TAppTSU
//- TAppTSU.node/kind tapp
//- TAppTSU param.0 _DependentTS  // Dependent struct not yet indexed.
//- TAppTSU param.1 TvarU
