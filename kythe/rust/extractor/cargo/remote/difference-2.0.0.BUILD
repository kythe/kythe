"""
cargo-raze crate build file.

DO NOT EDIT! Replaced on runs of cargo-raze
"""
package(default_visibility = [
  # Public for visibility by "@raze__crate__version//" targets.
  #
  # Prefer access through "//kythe/rust/extractor/cargo", which limits external
  # visibility to explicit Cargo.toml dependencies.
  "//visibility:public",
])

licenses([
  "notice", # MIT from expression "MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


rust_binary(
    # Prefix bin name to disambiguate from (probable) collision with lib name
    # N.B.: The exact form of this is subject to change.
    name = "cargo_bin_difference",
    deps = [
        # Binaries get an implicit dependency on their crate's lib
        ":difference",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/main.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "2.0.0",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
    ],
)


rust_library(
    name = "difference",
    crate_type = "lib",
    deps = [
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "2.0.0",
    tags = ["cargo-raze"],
    crate_features = [
        "default",
    ],
)

# Unsupported target "github-style" with type "example" omitted
# Unsupported target "line-by-line" with type "example" omitted
# Unsupported target "quickcheck" with type "test" omitted
# Unsupported target "underline-words" with type "example" omitted
