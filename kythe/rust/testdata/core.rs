#![feature(no_core)]
#![no_core]
#![cfg_attr(kythe_indexer_test, crate_name = "core")]
#![cfg_attr(kythe_indexer_test, stable(feature = "core", since = "1.0.0"))]

pub mod prelude {}

pub mod x {}
