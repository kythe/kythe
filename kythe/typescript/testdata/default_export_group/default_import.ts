// This test uses a default import, which should behave like importing
// a field named "default".

//- @obj ref/imports Def
import obj from './default_export';

//- @obj ref Def
obj.key;

//- @#0"default" ref/imports Def
import {default as obj2} from './default_export';
