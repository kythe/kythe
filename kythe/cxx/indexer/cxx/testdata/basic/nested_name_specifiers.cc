//- @a defines/binding A
struct a {
  //- @b defines/binding B
  struct b {};
};

//- @a ref A
//- @b ref B
a::b c;
