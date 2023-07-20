// This test imports the same value in a variety of names and ensures
// they all resolve to the same VName.

//- @"'./module'" ref/imports Mod
import './module';

//- @mod_imp defines/binding ModLocal
//- @"'./module'" ref/imports Mod
import * as mod_imp from './module';

// Importing from a module gets a VName that refers into the other module.
//- @value ref/imports Val
//- @"'./module'" ref/imports Mod
import {value} from './module';

// Importing from a module gets a VName that refers into the other module,.
//- @value ref/imports Val
//- @renamedValue defines/binding RenamedValue
//- RenamedValue.node/kind variable
//- RenamedValue.subkind import
//- RenamedValue aliases Val
import {value as renamedValue} from './module';

// Ensure the various names of the imported value link together.

//- @value ref Val
value;

//- @mod_imp ref ModLocal
//- @value ref Val
mod_imp.value;

//- @renamedValue ref Val
renamedValue;

//- @value ref/imports Val
import {value as exportedValue} from './export';
//- @local ref/imports Local
import {local} from './export';
//- @aliasedLocal ref/imports Local
import {aliasedLocal} from './export';

import * as allExport from './export';

// - @value ref Val
allExport.value;

// Importing a type from another module.
//- @MyType ref/imports MyType
import {MyType} from './module';
//- @MyType ref MyType
let x: MyType;
