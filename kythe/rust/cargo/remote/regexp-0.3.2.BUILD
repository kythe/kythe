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
  "notice", # MIT from expression "MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "ip_v4_addr" with type "test" omitted
# Unsupported target "ip_v6_addr" with type "test" omitted

rust_library(
    name = "regexp",
    crate_type = "lib",
    deps = [
        "@raze__lazy_static__1_4_0//:lazy_static",
        "@raze__regex__1_3_9//:regex",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.3.2",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

