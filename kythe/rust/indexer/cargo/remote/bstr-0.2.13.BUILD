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
  "notice", # MIT from expression "MIT OR Apache-2.0"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)



rust_library(
    name = "bstr",
    crate_type = "lib",
    deps = [
        "@raze__lazy_static__1_4_0//:lazy_static",
        "@raze__memchr__2_3_3//:memchr",
        "@raze__regex_automata__0_1_9//:regex_automata",
        "@raze__serde__1_0_111//:serde",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    data = glob(["**/*.dfa"]),
    version = "0.2.13",
    crate_features = [
        "default",
        "lazy_static",
        "regex-automata",
        "serde",
        "serde1",
        "serde1-nostd",
        "std",
        "unicode",
    ],
)

# Unsupported target "graphemes" with type "example" omitted
# Unsupported target "graphemes-std" with type "example" omitted
# Unsupported target "lines" with type "example" omitted
# Unsupported target "lines-std" with type "example" omitted
# Unsupported target "uppercase" with type "example" omitted
# Unsupported target "uppercase-std" with type "example" omitted
# Unsupported target "words" with type "example" omitted
# Unsupported target "words-std" with type "example" omitted
