export {}

//- @A defines/binding A
interface A {
  a: number;
}

//- @B defines/binding B
//- B.node/kind talias
//- B aliases A
type B = A;
