//- Top = vname("[0:0]","google","","kythe/rust/testdata/std.rs","rust") defines/implicit Std = vname("std","google","","","rust")
//- Std.node/kind package
#![no_std]
#![cfg_attr(kythe_indexer_test, crate_name = "std")]
#![cfg_attr(kythe_indexer_test, stable(feature = "rust1", since = "1.0.0"))]

//- @prelude defines/binding Prelude =
//- vname("std/~prelude","google","","","rust") Prelude childof Std
mod prelude {}

//- @x ref vname("core/~x","google","","","rust")
use core_lib::x;
