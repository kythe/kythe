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
test(
    3,
    {num: 3});

// Test computed function names by creating a constant string and checking that
// a name computed from it is properly referenced to its computed definition.
//- @#0"fn" defines/binding OFnStr
const fn = 'fn';
let o = {
  //- !{@"[fn]" defines/binding _}
  //- @fn ref OFnStr
  [fn]() {},
};

//- @fn ref OFnStr
o[fn]();

// Test arrow functions
//- @x defines/binding X
//- X.node/kind variable
let x: 3;

//- @#0x defines/binding ArrowX
//- ArrowX.node/kind variable
//- @#1x ref ArrowX
//- !{@#1x ref X}
test((x => x + 1)(3), undefined);

// Test default function arguments
//- @x ref X
function defaultArgument(a: number = x) {}

//- @NestedArray defines/binding NestedArray
type NestedArray = number[][];

//- @bindingTest defines/binding BF
//- @aa defines/binding AA
//- AA.node/kind variable
//- AA childof BF
//- BF param.0 AA
//- !{ @bb defines/binding _ }
//- @cc defines/binding CC
//- BF param.1 CC
//- @dd defines/binding DD
//- BF param.2 DD
//- @ee defines/binding EE
//- BF param.3 EE
//- @ff defines/binding FF
//- BF param.4 FF
//- @NestedArray ref NestedArray
function bindingTest({aa, bb: {cc, dd}}, [ee, [ff]]: NestedArray) {
  //- @dd ref/writes DD
  //- @ff ref FF
  dd = ff;
}

// Test function expressions introducing an Anon scope
//- @ev defines/binding Ev
//- @an defines/binding An
let ev, an;
(function() {
//- !{ @ev defines/binding _Ev2=Ev }
let ev;
//- @an ref An
an;
})();

//- AnonArrowDef defines _
//- AnonArrowDef.node/kind anchor
//- AnonArrowDef.loc/start @^"arg"
//- AnonArrowDef.loc/end @$"42"
(arg => 42)();

//- AnonFunctionDef defines _
//- AnonFunctionDef.node/kind anchor
//- AnonFunctionDef.loc/start @^"function"
(function() {
  return 42;
//- AnonFunctionDef.loc/end @$"}"
})();
