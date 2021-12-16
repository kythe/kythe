// Tests that we index explicit specializations of template variables.
//- @v defines/binding PrimaryTemplateV
template <typename T> T v;
//- @v defines/binding SpecIntV
//- SpecIntV specializes AppPTInt
//- AppPTInt.node/kind tapp
//- AppPTInt param.0 PrimaryTemplateV
//- AppPTInt param.1 vname("int#builtin",_,_,_,_)
template <> int v<int>;
template <> float v<float>;
