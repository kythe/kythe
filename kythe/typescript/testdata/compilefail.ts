// This file exercises what happens when you have a compilation error in a
// dependent module.  The tests skip checking this file itself (because it
// fails to typecheck, there's no indexing done) but it's imported by
// compilefail_import.ts (which doesn't type-check its inputs).
export type Bad = Undefined;
