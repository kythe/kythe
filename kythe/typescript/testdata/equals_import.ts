// This tests an equals import of an external module reference that uses the
// export equals syntax.

//- @obj ref/imports Def=vname("export =", _, _, "testdata/equals_export", _)
import obj = require('./equals_export');

//- @obj ref Def
obj.key;
