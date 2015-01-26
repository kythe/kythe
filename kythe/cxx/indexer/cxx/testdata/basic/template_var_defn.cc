// Checks that we index defns of variable templates.
//- @T defines TyvarT
template <typename T>
//- @T ref TyvarT
//- @y defines VarY
T y;
//- VarY.node/kind abs
//- VarYBody childof VarY
//- VarYBody.node/kind variable
//- VarYBody typed TyvarT
//- VarYBody.complete definition