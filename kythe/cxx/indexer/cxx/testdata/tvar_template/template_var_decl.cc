// Checks that we index decls of variable templates.
//- @T defines/binding TyvarT
template <typename T>
//- @T ref TyvarT
//- @y defines/binding VarY
extern T y;
//- VarY.node/kind abs
//- VarYBody childof VarY
//- VarYBody.node/kind variable
//- VarYBody typed TyvarT
//- VarYBody.complete incomplete