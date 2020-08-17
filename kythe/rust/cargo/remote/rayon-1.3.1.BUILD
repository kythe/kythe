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
  "notice", # Apache-2.0 from expression "Apache-2.0 OR MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "build-script-build" with type "custom-build" omitted
# Unsupported target "clones" with type "test" omitted
# Unsupported target "collect" with type "test" omitted
# Unsupported target "cpu_monitor" with type "example" omitted
# Unsupported target "cross-pool" with type "test" omitted
# Unsupported target "debug" with type "test" omitted
# Unsupported target "intersperse" with type "test" omitted
# Unsupported target "issue671" with type "test" omitted
# Unsupported target "issue671-unzip" with type "test" omitted
# Unsupported target "iter_panic" with type "test" omitted
# Unsupported target "named-threads" with type "test" omitted
# Unsupported target "octillion" with type "test" omitted
# Unsupported target "producer_split_at" with type "test" omitted

rust_library(
    name = "rayon",
    crate_type = "lib",
    deps = [
        "@raze__crossbeam_deque__0_7_3//:crossbeam_deque",
        "@raze__either__1_5_3//:either",
        "@raze__rayon_core__1_7_1//:rayon_core",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.3.1",
    tags = ["cargo-raze"],
    crate_features = [
    ],
)

# Unsupported target "sort-panic-safe" with type "test" omitted
# Unsupported target "str" with type "test" omitted
