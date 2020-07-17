// This test imports an "index" module, which is to say when
// you import a path "foo/bar" that refers to a directory, it
// may resolve to a file "foo/bar/index.[something]".

//- @"'./index_module_group'" ref/imports _ModRef=vname("module", _, _, "testdata/index_module_group/index", _)
import {foo} from './index_module_group';
