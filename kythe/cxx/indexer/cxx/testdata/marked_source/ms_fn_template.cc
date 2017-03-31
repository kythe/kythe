// We emit reasonable markup for function templates.

//- @f defines/binding AbsF
//- AbsF code ACRoot
//- ACRoot child.2 ACIdentToken
//- ACIdentToken.pre_text "f"
template <typename T> void f() {}

//- @f defines/binding FSpec
//- FSpec code SCRoot
//- SCRoot child.2 SCIdentToken
//- SCIdentToken.pre_text "f"
template <> void f<int>() {}

void g() {
  //- @f ref TAppFShort
  //- !{ InstF code _ }
  //- InstF instantiates TAppFShort
  //- TAppFShort param.0 AbsF
  f<short>();
}
