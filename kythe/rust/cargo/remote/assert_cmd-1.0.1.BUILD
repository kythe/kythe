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


# Unsupported target "assert" with type "test" omitted

rust_library(
    name = "assert_cmd",
    crate_type = "lib",
    deps = [
        "@raze__doc_comment__0_3_3//:doc_comment",
        "@raze__predicates__1_0_5//:predicates",
        "@raze__predicates_core__1_0_0//:predicates_core",
        "@raze__predicates_tree__1_0_0//:predicates_tree",
        "@raze__wait_timeout__0_2_0//:wait_timeout",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    data = ["README.md"],
    version = "1.0.1",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

rust_binary(
    # Prefix bin name to disambiguate from (probable) collision with lib name
    # N.B.: The exact form of this is subject to change.
    name = "cargo_bin_bin_fixture",
    deps = [
        # Binaries get an implicit dependency on their crate's lib
        ":assert_cmd",
        "@raze__doc_comment__0_3_3//:doc_comment",
        "@raze__predicates__1_0_5//:predicates",
        "@raze__predicates_core__1_0_0//:predicates_core",
        "@raze__predicates_tree__1_0_0//:predicates_tree",
        "@raze__wait_timeout__0_2_0//:wait_timeout",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/bin/bin_fixture.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    data = ["README.md"],
    version = "1.0.1",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "cargo" with type "test" omitted
# Unsupported target "example_fixture" with type "example" omitted
# Unsupported target "examples" with type "test" omitted
