// Checks that influence is recorded for simple function calls.

//- @a defines/binding VarA
//- @b defines/binding VarB
//- @f defines/binding FDecl
int f(int a, int b);

//- @c defines/binding VarC
//- @d defines/binding VarD
//- @f defines/binding FDefn
//- @f completes/uniquely FDecl
//- VarA influences VarC
//- VarB influences VarD
//- FDefn influences FDecl
int f(int c, int d) {
  //- VarC influences FDefn
  //- VarD influences FDefn
  return c + d;
}

void g() {
  //- @e defines/binding VarE
  //- @h defines/binding VarH
  int e = 0, h = 1,
  //- @i defines/binding VarI
      i = 2;
  //- @f ref FDefn
  //- VarE influences VarC
  //- VarH influences VarD
  //- FDefn influences VarI
  i = f(e, h);
}
