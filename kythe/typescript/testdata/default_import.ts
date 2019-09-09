// This test uses a default import, which should behave like importing
// a field named "default".

//- @obj ref/imports ObjDef=vname("default", _, _, "testdata/default_export", _)
//- @import defines/binding LocalObj=vname(_, _, _, "testdata/default_import", _)
//- LocalObj.node/kind variable
//- LocalObj.subkind import
//- LocalObj aliases ObjDef
import obj from './default_export';

//- @obj ref LocalObj
obj.key;

//- @#0"default" ref/imports ObjDef
//- @obj2 defines/binding LocalObj2=vname(_, _, _, "testdata/default_import", _)
//- LocalObj2.node/kind variable
//- LocalObj2.subkind import
//- LocalObj2 aliases ObjDef
import {default as obj2} from './default_export';
