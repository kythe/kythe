// This test verifies that the "export {...}" syntax is tagged as
// defining a module.

//- @"export" defines/binding vname("module", _, _, "testdata/reexporter", _)
export {value} from './module';
