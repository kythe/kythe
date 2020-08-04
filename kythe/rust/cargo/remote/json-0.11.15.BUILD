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



rust_library(
    name = "json",
    crate_type = "lib",
    deps = [
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.11.15",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "json_checker" with type "test" omitted
# Unsupported target "log" with type "bench" omitted
# Unsupported target "number" with type "test" omitted
# Unsupported target "parse" with type "test" omitted
# Unsupported target "print_dec" with type "test" omitted
# Unsupported target "stringify" with type "test" omitted
# Unsupported target "value" with type "test" omitted
