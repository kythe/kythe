// Tests the combination of overloading and specialization on function
// templates.
//- @f defines/binding FnF
template <typename T> void f(T t) { }
//- @f defines/binding FnFInt
template <> void f(int) { }
//- @f defines/binding FnFPtr
template <typename T> void f(T* t) { }
//- FnFInt specializes TAppFnF
//- TAppFnF param.0 FnF