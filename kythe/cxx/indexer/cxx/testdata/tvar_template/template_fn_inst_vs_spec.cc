// Checks that instantiates/specializes are applied to function templates.
// Note that function template partial specialization is not allowed. We will
// still see instantiates edges, but they will point to either the primary
// template or to the total specialization used.

//- @f defines/binding PrimaryF
template <typename T> bool f(T* arg);
//- @f defines/binding TotalF
template <> bool f(int* arg);

//- @f ref TotalF
//- TotalF specializes PrimaryFInt
//- TotalF instantiates PrimaryFInt
//- PrimaryFInt param.0 PrimaryF
bool t = f((int*)nullptr);

//- @f ref PrimaryFDouble
//- ImpPrimaryFDouble.node/kind function
//- ImpPrimaryFDouble specializes PrimaryFDouble
//- ImpPrimaryFDouble instantiates PrimaryFDouble
//- PrimaryFDouble param.0 PrimaryF
bool s = f((double*)nullptr);
