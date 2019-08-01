// This test uses a default import, which should behave like importing
// a field named "default".

//- @obj ref/imports Def=vname("default", _, _, "testdata/default_export", _)
//- LocalObj=vname("obj", _, _, "testdata/default_import", _).node/kind name
//- Def generates LocalObj
import obj from './default_export';

//- @obj ref LocalObj
obj.key;

//- @obj2 ref/imports Def
//- LocalObj2=vname("obj2", _, _, "testdata/default_import", _).node/kind name
//- Def generates LocalObj2
import {default as obj2} from './default_export';
