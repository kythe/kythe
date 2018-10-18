export {}

//- @A defines/binding _A
interface A {
  a: number;
}

// TODO: see TODO in visitTypeAlias.
// B aliases A

//- @B defines/binding B
//- B.node/kind talias
type B = A;
