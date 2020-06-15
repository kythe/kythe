// Checks that variable references in a function are blamed on that function.

//- @f defines/binding FnF
void f() {
  //- @x defines/binding VarX
	int x;
  //- @x ref VarX
  //- @x childof FnF
	x = 3;
}
