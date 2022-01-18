// Tests overloads of template functions.
//- @T defines/binding FNoPtrT
template <typename T>
//- @f defines/binding FNoPtr
void f(T t) { }
//- @T defines/binding FPtrT
template <typename T>
//- @f defines/binding FPtr
void f(T* t) { }
//- FNoPtr typed TAppFnT
//- TAppFnT param.2 FNoPtrT
//- FPtr typed TAppFnTPtr
//- TAppFnTPtr param.2 FPtrTTy
//- FPtrTTy param.1 FPtrT
