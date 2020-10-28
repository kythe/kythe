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
  "restricted", # BSD-2-Clause from expression "BSD-2-Clause"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)



rust_library(
    name = "cloudabi",
    crate_type = "lib",
    deps = [
        "@raze__bitflags__1_2_1//:bitflags",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "cloudabi.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.0.3",
    tags = ["cargo-raze"],
    crate_features = [
        "bitflags",
        "default",
    ],
)

