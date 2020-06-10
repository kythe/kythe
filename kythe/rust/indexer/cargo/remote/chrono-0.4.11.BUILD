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


# Unsupported target "chrono" with type "bench" omitted

rust_library(
    name = "chrono",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__num_integer__0_1_42//:num_integer",
        "@raze__num_traits__0_2_11//:num_traits",
        "@raze__time__0_1_43//:time",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.4.11",
    crate_features = [
        "clock",
        "default",
        "std",
        "time",
    ],
)

# Unsupported target "serde" with type "bench" omitted
# Unsupported target "wasm" with type "test" omitted
