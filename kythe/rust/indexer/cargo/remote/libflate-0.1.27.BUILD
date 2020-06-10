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
  "notice", # "MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "flate" with type "example" omitted

rust_library(
    name = "libflate",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2015",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__adler32__1_0_4//:adler32",
        "@raze__crc32fast__1_2_0//:crc32fast",
        "@raze__rle_decode_fast__1_0_1//:rle_decode_fast",
        "@raze__take_mut__0_2_2//:take_mut",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.1.27",
    crate_features = [
    ],
)

