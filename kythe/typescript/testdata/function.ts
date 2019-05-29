export {}

// Declare a type for the function to return, to check indexing through
// the return type.
//- @Num defines/binding Num
interface Num {
  //- @#0num defines/binding _NumAttr
  num: number;
}

//- TestDef defines F
//- TestDef.node/kind anchor
//- @test defines/binding F
//- F.node/kind function
//- @a defines/binding ParamA
//- ParamA.node/kind variable
//- F param.0 ParamA
//- @#0Num ref Num
//- @#1Num ref Num
//- TestDef.loc/start @^function
function test(a: number, num: Num): Num {
  // TODO(evanm): it would be nice for @num ref NumAttr.  TypeScript seems to
  // know they're linked, in that if you rename 'num' above it knows to rename
  // this one.  However, it may be the case that they are not linked beyond
  // renaming -- experimentally, I found that if you have some doc comment on
  // the def'n of 'num' above, it doesn't show up when you hover 'num' below.
  //- @a ref ParamA
  return {num: a};
  //- TestDef.loc/end @$"}"
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

//- @x ref X
function defaultArgument(a: number = x) {}
