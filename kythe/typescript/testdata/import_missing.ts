// Perhaps surprisingly, TypeScript does not error on missing side-effect
// imports like the below.  The rationale is something about how they
// bring no symbols into the current module so there's nothing to check.
// We include one of these in the test suite to ensure we don't fail on it.

import 'nosuchmodule';
