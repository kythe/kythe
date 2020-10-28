export {}

//- @A defines/binding A
interface A {
  a: number;
}

//- @B defines/binding B
//- B.node/kind talias
//- B aliases A
type B = A;

// Test that aliases to types with type arguments do not emit a reference edge
// to anything. VName signatures of types with arguments are not qualified with
// the arguments.
//- @T1 defines/binding T1
//- !{ T1 aliases _ }
type T1 = Array<string>;
//- @T2 defines/binding T2
//- !{ T2 aliases _ }
type T2 = Array<number>;
