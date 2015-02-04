// Documents [expr.prim.lambda].
void f3() {
  //- @float ref FloatType
  float x;
  //- @r defines VarR VarR typed RefFloat
  float &r = x;
  //- @cr defines VarCR VarCR typed ConstRefFloat
  const float &cr = x;
  (void) [=] {
    // x, r are not captured ("appearance in a decltype operand is not an
    // odr-use")
    //- @decltype ref FloatType
    //- @y1 defines VarY1 VarY1 typed FloatType
    decltype(x) y1;
    // ("because this lambda is not mutable and x is an lvalue")
    //- @decltype ref ConstRefFloat
    decltype((x)) y2 = y1;
    // ("transformation not considered")
    //- @decltype ref RefFloat
    decltype(r) r1 = y1;
    //- @decltype ref ConstRefFloat
    decltype((r)) r2 = y2;
  };
}
