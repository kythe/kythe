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
  "notice", # MIT from expression "MIT OR (Apache-2.0 AND BSD-2-Clause)"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "array_queue" with type "test" omitted

rust_library(
    name = "crossbeam_queue",
    crate_type = "lib",
    deps = [
        "@raze__cfg_if__0_1_10//:cfg_if",
        "@raze__crossbeam_utils__0_7_2//:crossbeam_utils",
        "@raze__maybe_uninit__2_0_0//:maybe_uninit",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.2.3",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
        "std",
    ],
)

# Unsupported target "seg_queue" with type "test" omitted
