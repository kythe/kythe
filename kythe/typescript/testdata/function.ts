export {}

//- @test defines/binding F
//- F.node/kind function
function test() {
  // Check that we index function body:
  //- @x defines/binding X
  //- X.node/kind variable
  let x = 1;
}

//- @test ref F
test();
