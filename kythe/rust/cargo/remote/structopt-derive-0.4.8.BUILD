"""
cargo-raze crate build file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""
package(default_visibility = [
  # Public for visibility by "@raze__crate__version//" targets.
  #
  # Prefer access through "//kythe/rust/cargo", which limits external
  # visibility to explicit Cargo.toml dependencies.
  "//visibility:public",
])

licenses([
  "notice", # Apache-2.0 from expression "Apache-2.0 OR MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)



rust_library(
    name = "structopt_derive",
    crate_type = "proc-macro",
    deps = [
        "@raze__heck__0_3_1//:heck",
        "@raze__proc_macro_error__1_0_3//:proc_macro_error",
        "@raze__proc_macro2__1_0_19//:proc_macro2",
        "@raze__quote__1_0_7//:quote",
        "@raze__syn__1_0_36//:syn",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.4.8",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

