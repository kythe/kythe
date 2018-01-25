// Tests that tag declaration embedded in a declarator
// are properly parented.

//- @outer defines/binding Outer
struct outer {
  //- @inner ref Inner
  //- Inner.node/kind tnominal
  //- Inner childof Outer
  struct inner* array[1];
};
