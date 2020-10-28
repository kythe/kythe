/** @fileoverview Verify that reexporting indexed correctly. */

export {
  //- @#0Class ref Class
  //- @ClassAlias ref Class
  Class as ClassAlias,
  //- @#0CONSTANT ref Constant
  //- @CONSTANT_ALIAS ref Constant
  CONSTANT as CONSTANT_ALIAS,
  //- @#0Enum ref Enum
  //- @EnumAlias ref Enum
  Enum as EnumAlias,
  //- @#0Interface ref Interface
  //- @InterfaceAlias ref Interface
  Interface as InterfaceAlias,
  //- @#0someFunction ref SomeFunction
  //- @someFunctionAlias ref SomeFunction
  someFunction as someFunctionAlias,
  //- @#0Type ref Type
  //- @TypeAlias ref Type
  Type as TypeAlias}
//- @"'./main'" ref/imports vname("module", _, _, "testdata/reexport_group/main", _)
from './main';
