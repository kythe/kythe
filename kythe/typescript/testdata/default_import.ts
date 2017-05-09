// This test uses a default import, which should behave like importing
// a field named "default".

//- @obj ref/imports Def=VName("default", _, _, "testdata/default_export", _)
import obj from './default_export';

//- @obj ref Def
obj.key;

//- @obj2 ref/imports Def
import {default as obj2} from './default_export';
