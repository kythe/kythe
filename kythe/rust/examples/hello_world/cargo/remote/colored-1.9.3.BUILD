"""
cargo-raze crate build file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""
package(default_visibility = [
  # Public for visibility by "@raze__crate__version//" targets.
  #
  # Prefer access through "//kythe/rust/examples/hello_world/cargo", which limits external
  # visibility to explicit Cargo.toml dependencies.
  "//visibility:public",
])

licenses([
  "reciprocal", # "MPL-2.0"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "ansi_term_compat" with type "test" omitted

rust_library(
    name = "colored",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__atty__0_2_14//:atty",
        "@raze__lazy_static__1_4_0//:lazy_static",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.9.3",
    crate_features = [
    ],
)

# Unsupported target "control" with type "example" omitted
# Unsupported target "dynamic_colors" with type "example" omitted
# Unsupported target "most_simple" with type "example" omitted
# Unsupported target "nested_colors" with type "example" omitted
