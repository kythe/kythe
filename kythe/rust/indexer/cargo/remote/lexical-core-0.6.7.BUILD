"""
cargo-raze crate build file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""
package(default_visibility = [
  # Public for visibility by "@raze__crate__version//" targets.
  #
  # Prefer access through "//kythe/rust/indexer/cargo", which limits external
  # visibility to explicit Cargo.toml dependencies.
  "//visibility:public",
])

licenses([
  "notice", # "MIT,Apache-2.0"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "build-script-build" with type "custom-build" omitted

rust_library(
    name = "lexical_core",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__arrayvec__0_4_12//:arrayvec",
        "@raze__bitflags__1_2_1//:bitflags",
        "@raze__cfg_if__0_1_9//:cfg_if",
        "@raze__ryu__1_0_5//:ryu",
        "@raze__static_assertions__0_3_4//:static_assertions",
    ],
    rustc_flags = [
        "--cap-lints=allow",
        "--cfg=has_range_bounds",
        "--cfg=has_slice_index",
        "--cfg=has_full_range_inclusive",
        "--cfg=has_const_index",
        "--cfg=has_i128",
        "--cfg=has_ops_bound",
        "--cfg=has_pointer_methods",
        "--cfg=has_range_inclusive",
        "--cfg=limb_width_64",
    ],
    version = "0.6.7",
    crate_features = [
        "arrayvec",
        "correct",
        "default",
        "ryu",
        "static_assertions",
        "std",
        "table",
    ],
)

