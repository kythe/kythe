// Verify that importing from a failing-to-compile module doesn't cause this
// module to also fail compilation.  This indirectly verifies that we don't
// type-check inputs other than the ones that were requested.
import {Bad} from './compilefail';
let x: Bad;
