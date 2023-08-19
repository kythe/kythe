/**
 * @fileoverview Test to ensure that indexer produces ref/call edges and all data needed for callgraph.
 * See https://kythe.io/docs/schema/callgraph.html.
 */

// clang-format off
//- FileInitFunc=vname("fileInit:synthetic", _, _, "testdata/refcall", "typescript").node/kind function
//- FileInitDef defines FileInitFunc
//- FileInitDef.node/kind anchor
//- FileInitDef.loc/start 0
//- FileInitDef.loc/end 0
// clang-format on

//- @Square defines/binding Square
//- Square.node/kind function
class Square {
  //- @getWidth defines/binding GetWidth
  getWidth(): number {
    return 42;
  }

  //- @getArea defines/binding GetArea
  getArea() {
    //- GetWidthCall=@"this.getWidth()" ref/call GetWidth
    //- GetWidthCall childof GetArea
    this.getWidth() ** 2;
  }
}

//- @"new Square()" ref/call Square
const square = new Square();

//- GetAreaCallOne=@"square.getArea()" ref/call GetArea
//- GetAreaCallOne childof FileInitFunc
square.getArea();

//- @doNothing defines/binding DoNothing
function doNothing() {
  //- GetAreaCallTwo=@"square.getArea()" ref/call GetArea
  //- GetAreaCallTwo childof DoNothing
  square.getArea();
}

//- DoNothingCall=@"doNothing()" ref/call DoNothing
//- DoNothingCall childof FileInitFunc
doNothing();

//- GetAreaCallInAnonArrow=@"square.getArea()" ref/call GetArea
//- GetAreaCallInAnonArrow childof AnonArrow
//- AnonArrowCall ref/call AnonArrow
//- AnonArrowCall childof FileInitFunc
(() => square.getArea())();

//- AnonFunctionCall ref/call AnonFunction
//- AnonFunctionCall childof FileInitFunc
(function() {
  //- GetAreaCallInAnonFunction=@"square.getArea()" ref/call GetArea
  //- GetAreaCallInAnonFunction childof AnonFunction
  square.getArea();
})();
