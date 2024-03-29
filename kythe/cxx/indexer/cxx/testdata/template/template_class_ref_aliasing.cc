//- @S defines/binding SPrim
//- @S defines/binding S1
template <typename T, typename U> struct S {};
//- @S defines/binding S2
//- S2.node/kind record  // avoid implicit constructors
template <typename T> struct S<T, int> {};
//- @S defines/binding S3
//- S3.node/kind record
template <typename T> struct S<T, float> {};
//- @S defines/binding S4
//- S4.node/kind record
template <typename U> struct S<int, U> {};
//- @S defines/binding S5
//- S5.node/kind record
template <typename U> struct S<float, U> {};
//- @S defines/binding S6
//- S6.node/kind record
template <> struct S<char, char> {};

//- @S ref S1
S<void *, void *> s1;
//- @S ref S2
S<void *, int> s2;
//- @S ref S3
S<void *, float> s3;
//- @S ref S4
S<int, void *> s4;
//- @S ref S5
S<float, void *> s5;
//- @S ref S6
S<char, char> s6;

template <typename T, typename U> void f() {
  //- @S ref SPrim
  S<T, U> s;
}
