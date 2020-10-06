#include "exclude_this_file.h"
#include "include_this_file.h"

//- @inc defines/binding VarInc
Included<int> inc;
//- @exc defines/binding VarExc
Excluded<int> exc;

//- VarInc typed TAppIncInt
//- TAppIncInt.node/kind tapp
//- TAppIncInt param.0 _TemplateIncluded
//- _ImpInc instantiates TAppIncInt

//- VarExc typed TAppExcInt
//- TAppExcInt.node/kind tapp
//- TAppExcInt param.0 _TemplateExcluded
//- !{_ImpExc instantiates TAppExcInt}

INC_MACRO
EXC_MACRO

//- @inc_macro defines/binding VarIncMacro
IncludedMacro<int> inc_macro;
//- @exc_macro defines/binding VarExcMacro
ExcludedMacro<int> exc_macro;

//- VarIncMacro typed TAppIncMacroInt
//- TAppIncMacroInt.node/kind tapp
//- TAppIncMacroInt param.0 _TemplateIncludedMacro
//- _ImpMacroInc instantiates TAppIncMacroInt

//- VarExcMacro typed TAppExcMacroInt
//- TAppExcMacroInt.node/kind tapp
//- TAppExcMacroInt param.0 _TemplateExcludedMacro
//- !{_ExcMacroInc instantiates TAppExcMacroInt}

void f() {
  //- @IF ref IncFun
  IF(1);
  //- @EF ref ExcFun
  EF(1);
}

//- _ImpIncFun instantiates IncFun
//- !{_ImpExcFun instantiates ExcFun}

//- @inc_var ref _IncVar
int x = inc_var<int>;
//- @exc_var ref _ExcVar
int y = exc_var<int>;
