// This import is actually importing from a path within the
// fake-genfiles/ subtree, but the tsconfig.json specifies that
// fake-genfiles/ should be treated as a root directory for path
// lookups.  This is used to model the parallel trees of
// source vs bazel-genfiles etc.

// Note that the path of the import should be the module name,
// which is "testdata/alt_module", and *not* the on-disk path
// "testdata/fake-genfiles/testdata/alt_module".
// See the discussion of module names in README.md.

//- @exported ref/imports _Alt=vname("testdata/alt_module/exported", _, _, "testdata/alt_module", _)
//- @"'./alt_module'" ref/imports _ModRef=vname("testdata/alt_module", _, _, "testdata/alt_module", _)
import {exported} from './alt_module';
