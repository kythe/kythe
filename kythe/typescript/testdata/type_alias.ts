export {}

//- @A defines/binding A
interface A {
  a: number;
}

//- @B defines/binding B
//- B.node/kind talias
//- B aliases A
type B = A;

//- @T1 defines/binding T1
type T1 = Array<string>;
//- @T2 defines/binding T2
//- !{ T2 aliases T1 }
type T2 = Array<string>;
