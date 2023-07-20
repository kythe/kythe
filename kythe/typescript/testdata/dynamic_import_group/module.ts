//- Mod=vname("module", _, _, "testdata/dynamic_import_group/module", _).node/kind record

//- @StuffDoer defines/binding StuffDoer
//- StuffDoer.node/kind record
export class StuffDoer {
    doStuff() {}
}

//- @CONSTANT defines/binding Constant
export const CONSTANT = 1;

//- @doStuff defines/binding DoStuff
export function doStuff() {}

//- @Enum defines/binding Enum
export enum Enum {}