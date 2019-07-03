// Syntax 1: exporting a value from another module.
//- @"'./module'" ref/imports vname("module", _, _, "testdata/module", _)
export {value} from './module';

// Syntax 2: a bare "export" statement.
//- @local defines/binding Local
const local = 3;
//- @local ref Local
export {local};
//- @local ref Local
//- @aliasedLocal ref Local
export {local as aliasedLocal};
