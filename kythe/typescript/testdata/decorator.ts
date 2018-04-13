export {}

// Checks decorators as annotations on classes, methods, and parameters.
// Note that decorators are not valid in all potential syntactic positions,
// just these.

//- @decor defines/binding Decor
declare var decor: any;

//- @value defines/binding Value
const value = 3;

//- @decor ref Decor
//- @value ref Value
@decor(value)
class C {
  //- @decor ref Decor
  //- @value ref Value
  @decor(value)
  //- @decor ref Decor
  //- @value ref Value
  method(@decor(value) param) {
  }
}
