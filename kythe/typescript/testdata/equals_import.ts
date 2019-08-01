// This tests an equals import of an external module reference that uses the
// export equals syntax.

//- @obj ref/imports Def=vname("export=", _, _, "testdata/equals_export", _)
//- LocalObj=vname("obj", _, _, "testdata/equals_import", _).node/kind name
//- Def named LocalObj
import obj = require('./equals_export');

//- @obj ref LocalObj
obj.key;
