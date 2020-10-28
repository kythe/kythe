// This test uses a default import, which should behave like importing
// a field named "default".

//- @obj ref/imports Def
//- @import defines/binding LocalObj
//- LocalObj.node/kind variable
//- LocalObj.subkind import
//- LocalObj aliases ObjDef
import obj from './default_export';

//- @obj ref LocalObj
obj.key;

//- @#0"default" ref/imports Def
//- @obj2 defines/binding LocalObj2
//- LocalObj2.node/kind variable
//- LocalObj2.subkind import
//- LocalObj2 aliases ObjDef
import {default as obj2} from './default_export';
