// Tests the combination of overloading and specialization on function
// templates.
//- @f defines FnF
template <typename T> void f(T t) { }
//- @f defines FnFInt
template <> void f(int) { }
//- @f defines FnFPtr
template <typename T> void f(T* t) { }
//- FnFInt specializes TAppFnF
//- TAppFnF param.0 FnF