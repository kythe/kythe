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
  "notice", # BSD-3-Clause from expression "BSD-3-Clause"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)



rust_library(
    name = "argh",
    crate_type = "lib",
    deps = [
        "@raze__argh_shared__0_1_1//:argh_shared",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    proc_macro_deps = [
        "@raze__argh_derive__0_1_1//:argh_derive",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.1.3",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "lib" with type "test" omitted
