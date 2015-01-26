// Checks that function overloads are recorded.
//- @F defines FnFI
void F(int X) { }
//- @F defines FnFF
void F(float Y) { }
//- FnFI param.0 IntX
//- FnFF param.0 FloatY
//- IntX named vname("X:F#n",_,_,_,_)
//- FloatY named vname("Y:F#n",_,_,_,_)
