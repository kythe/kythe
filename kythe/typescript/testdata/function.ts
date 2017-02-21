export {}

//- @test defines/binding F
//- F.node/kind function
//- @a defines/binding ParamA
//- ParamA.node/kind variable
//- F param.0 ParamA
function test(a: number) {
  // Check that we index function body:
  //- @x defines/binding X
  //- X.node/kind variable
  let x = 1;

  // Check indexing through "return" statements.
  //- @a ref ParamA
  return a;
}

//- @test ref F
test(3);
