// This test imports the same value in a variety of names and ensures
// they all resolve to the same VName.

//- @"'./module'" ref/imports ModRef=VName("module", _, _, "testdata/module", _)
import './module';

//- @mod_imp defines/binding ModLocal
//- @"'./module'" ref/imports ModRef=VName("module", _, _, "testdata/module", _)
import * as mod_imp from './module';

// Importing from a module gets a VName that refers into the other module.
//- @value ref/imports Val=VName(_, _, _, "testdata/module", _)
//- @"'./module'" ref/imports ModRef
//- LocalValue=VName("value", _, _, "testdata/import", _).node/kind name
//- Val generates LocalValue
import {value} from './module';

// Importing from a module gets a VName that refers into the other module,.
//- @value ref/imports Val
//- @renamedValue ref/imports Val
//- RenamedValue=VName("renamedValue", _, _, "testdata/import", _).node/kind name
//- Val generates RenamedValue
import {value as renamedValue} from './module';

// Ensure the various names of the imported value link together.

//- @value ref LocalValue
value;

//- @mod_imp ref ModLocal
//- @value ref Val
mod_imp.value;

//- @renamedValue ref RenamedValue
renamedValue;

//- @value ref/imports Val
import {value as exportedValue} from './export';
//- @local ref/imports Local
import {local} from './export';
//- @aliasedLocal ref/imports Local
import {aliasedLocal} from './export';

// Importing a type from another module.
//- @MyType ref/imports MyType
//- LocalMyType=VName("MyType#type", _, _, "testdata/import", _).node/kind name
//- MyType generates LocalMyType
import {MyType} from './module';
//- @MyType ref LocalMyType
let x: MyType;
