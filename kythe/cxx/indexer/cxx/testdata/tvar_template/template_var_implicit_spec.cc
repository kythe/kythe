// Tests that we index implicit specializations of template variables.
//- @v defines/binding PrimaryTemplateV
template <typename T> T v;
//- @v ref SpecIntV
//- SpecIntV instantiates AppPTInt
//- SpecIntV specializes AppPTInt
//- AppPTInt.node/kind tapp
//- AppPTInt param.0 PrimaryTemplateV
//- AppPTInt param.1 vname("int#builtin",_,_,_,_)
int w = v<int>;
