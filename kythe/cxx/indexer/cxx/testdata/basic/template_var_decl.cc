// Checks that we index decls of variable templates.
//- @T defines TyvarT
template <typename T>
//- @T ref TyvarT
//- @y defines VarY
extern T y;
//- VarY.node/kind abs
//- VarYBody childof VarY
//- VarYBody.node/kind variable
//- VarYBody typed TyvarT
//- VarYBody.complete incomplete