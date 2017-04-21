// This test verifies that importing from a d.ts produces a VName that
// doesn't mention the d.ts.

import * as mod from './declaration';

//- @decl ref Val=VName(_, _, _, "testdata/declaration", _)
mod.decl;
