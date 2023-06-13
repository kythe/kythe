// Checks that we index decls of variable templates.
//- @T defines/binding TyvarT
template <typename T>
//- @T ref TyvarT
//- @y defines/binding VarYBody
extern T y;
//- VarYBody.node/kind variable
//- VarYBody typed TyvarT
//- VarYBody.complete incomplete