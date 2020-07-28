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


# Unsupported target "cc_env" with type "test" omitted

rust_library(
    name = "gcc",
    crate_type = "lib",
    deps = [
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.3.55",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

rust_binary(
    # Prefix bin name to disambiguate from (probable) collision with lib name
    # N.B.: The exact form of this is subject to change.
    name = "cargo_bin_gcc_shim",
    deps = [
        # Binaries get an implicit dependency on their crate's lib
        ":gcc",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/bin/gcc-shim.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.3.55",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "test" with type "test" omitted
