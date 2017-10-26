// We index C++17 structed bindings.

void g() {
  int s[2] = {1, 2};

  //- @x defines/binding GLocalX
  //- @y defines/binding GLocalY
  const auto& [ x, y ] = s;

  //- @x ref GLocalX
  //- @y ref GLocalY
  s[0] == x && s[1] == y;
}

void f() {
  struct {
    //- @a defines/binding FieldA
    //- @b defines/binding FieldB
    int a, b;
  } s = {1, 2};

  //- @x defines/binding FLocalX
  //- @y defines/binding FLocalY
  //- !{ @x ref FieldA }
  //- !{ @y ref FieldB }
  const auto& [ x, y ] = s;

  //- @x ref FLocalX
  //- @y ref FLocalY
  //- @a ref FieldA
  //- @b ref FieldB
  //- !{ @x ref FieldA }
  //- !{ @y ref FieldB }
  s.a == x && s.b == y;
}
