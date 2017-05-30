export {}

// Declare a type for the function to return, to check indexing through
// the return type.
//- @Num defines/binding Num
interface Num {
  num: number;
}

//- @test defines/binding F
//- F.node/kind function
//- @a defines/binding ParamA
//- ParamA.node/kind variable
//- F param.0 ParamA
//- @#0Num ref Num
//- @#1Num ref Num
function test(a: number, num: Num): Num {
  // Check indexing function body and indexing through "return" statements.
  //- @a ref ParamA
  return {num: a};
}

//- @test ref F
test(3, {num: 3});

//- @x defines/binding X
//- X.node/kind variable
let x: 3;

//- @#0x defines/binding ArrowX
//- ArrowX.node/kind variable
//- @#1x ref ArrowX
//- !{@#1x ref X}
test((x => x + 1)(3), undefined);
