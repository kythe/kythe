// Checks that we index defns of variable templates.
//- @T defines/binding TyvarT
template <typename T>
//- @T ref TyvarT
//- @y defines/binding VarY
T y;
//- VarY.node/kind abs
//- VarYBody childof VarY
//- VarYBody.node/kind variable
//- VarYBody typed TyvarT
//- VarYBody.complete definition