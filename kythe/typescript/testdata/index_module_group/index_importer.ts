// This test imports an "index" module, which is to say when
// you import a path "foo/bar" that refers to a directory, it
// may resolve to a file "foo/bar/index.[something]".

//- @"'./index_module'" ref/imports Mod
import {foo} from './index_module';
