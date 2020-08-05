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
  "unencumbered", # Unlicense from expression "Unlicense OR MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "build" with type "bench" omitted

rust_library(
    name = "fst",
    crate_type = "lib",
    deps = [
        "@raze__byteorder__1_3_4//:byteorder",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.3.5",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "search" with type "bench" omitted
# Unsupported target "test" with type "test" omitted
