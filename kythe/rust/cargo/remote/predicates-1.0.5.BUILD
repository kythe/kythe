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
  "notice", # MIT from expression "MIT OR Apache-2.0"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "case_tree" with type "example" omitted

rust_library(
    name = "predicates",
    crate_type = "lib",
    deps = [
        "@raze__difference__2_0_0//:difference",
        "@raze__float_cmp__0_8_0//:float_cmp",
        "@raze__normalize_line_endings__0_3_0//:normalize_line_endings",
        "@raze__predicates_core__1_0_0//:predicates_core",
        "@raze__regex__1_3_9//:regex",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.0.5",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
        "difference",
        "float-cmp",
        "normalize-line-endings",
        "regex",
    ],
)

