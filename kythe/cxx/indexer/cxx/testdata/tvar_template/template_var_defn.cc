// Checks that we index defns of variable templates.
//- @T defines/binding TyvarT
template <typename T>
//- @T ref TyvarT
//- @y defines/binding VarYBody
T y;
//- VarYBody.node/kind variable
//- VarYBody typed TyvarT
//- VarYBody.complete definition