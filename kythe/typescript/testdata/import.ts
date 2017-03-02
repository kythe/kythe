// This test imports the same value in a variety of names and ensures
// they all resolve to the same VName.

//- @mod_imp defines/binding ModLocal
//- @"'./module'" ref/imports ModRef=VName("module", _, _, "testdata/module", _)
import * as mod_imp from './module';

// Importing from a module gets a VName that refers into the other module.
//- @value ref Val=VName(_, _, _, "testdata/module", _)
//- @"'./module'" ref/imports ModRef
import {value} from './module';

// Import and rename a value and ensure all of the references link.
//- @value ref Val
//- @renamedValue ref Val
import {value as renamedValue} from './module';

//- @value ref Val
value;

//- @mod_imp ref ModLocal
//- @value ref Val
mod_imp.value;

//- @renamedValue ref Val
renamedValue;
