/**
 * @fileoverview Test to ensure that indexer produces ref/call edges and all data needed for callgraph.
 * See https://kythe.io/docs/schema/callgraph.html.
 */

// clang-format off
//- FileInitFunc=vname("fileInit:synthetic", _, _, "testdata/refcall_id", "typescript").node/kind function
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
    //- GetWidthCall=@getWidth ref/call GetWidth
    //- GetWidthCall childof GetArea
    this.getWidth() ** 2;
  }
}

//- @Square ref/call Square
const square = new Square();

//- GetAreaCallOne=@getArea ref/call GetArea
//- GetAreaCallOne childof FileInitFunc
square.getArea();

//- @doNothing defines/binding DoNothing
function doNothing() {
  //- GetAreaCallTwo=@getArea ref/call GetArea
  //- GetAreaCallTwo childof DoNothing
  square.getArea();
}

//- DoNothingCall=@doNothing ref/call DoNothing
//- DoNothingCall childof FileInitFunc
doNothing();
