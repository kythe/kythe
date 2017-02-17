export {}

//- @x defines/binding X
let x = 3;

//- @y defines/binding AY
function a(y) {
  //- @x defines/binding AX
  let x = 3;
  {
    //- @x defines/binding AX2
    let x = 3;
    //- @x ref AX2
    //- !{@x ref X}
    //- !{@x ref AX}
    x;
  }
  {
    //- @x defines/binding AX3
    let x = 3;
    //- @y defines/binding AY2
    let y = 3;
    //- @x ref AX3
    //- !{@x ref AX2}
    x;
  }
  //- @x ref AX
  x;
  //- @y ref AY
  //- !{@y ref AY2}
  y;
}

// This x refers to the outer scope's x.
//- @x ref X
x;
