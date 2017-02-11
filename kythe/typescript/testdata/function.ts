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
}

//- @test ref F
test(3);
