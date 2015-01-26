// Tests overloads of template functions.
//- @T defines FNoPtrT
template <typename T>
//- @f defines FNoPtr
void f(T t) { }
//- @T defines FPtrT
template <typename T>
//- @f defines FPtr
void f(T* t) { }
//- FNoPtrFn childof FNoPtr
//- FPtrFn childof FPtr
//- FNoPtrFn typed TAppFnT
//- TAppFnT param.2 FNoPtrT
//- FPtrFn typed TAppFnTPtr
//- TAppFnTPtr param.2 FPtrTTy
//- FPtrTTy param.1 FPtrT
