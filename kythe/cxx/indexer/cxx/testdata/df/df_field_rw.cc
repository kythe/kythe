// Field writes are distinguished from reads.

struct S {
  //- @f defines/binding FieldF
  int f;
};

struct T {
  //- @g defines/binding FieldG
  S g;
};

int f() {
  S s;
  //- @f ref/writes FieldF
  s.f = 3;
  T t;
  //- @f ref/writes FieldF
  //- @g ref FieldG
  t.g.f = 3;
  //- @f ref FieldF
  return s.f +
    //- @g ref FieldG
    //- @f ref FieldF
    t.g.f;
}
