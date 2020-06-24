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
  "unencumbered", # Unlicense from expression "Unlicense OR MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "bench" with type "bench" omitted
# Unsupported target "cookbook-read-basic" with type "example" omitted
# Unsupported target "cookbook-read-colon" with type "example" omitted
# Unsupported target "cookbook-read-no-headers" with type "example" omitted
# Unsupported target "cookbook-read-serde" with type "example" omitted
# Unsupported target "cookbook-write-basic" with type "example" omitted
# Unsupported target "cookbook-write-serde" with type "example" omitted

rust_library(
    name = "csv",
    crate_type = "lib",
    deps = [
        "@raze__bstr__0_2_13//:bstr",
        "@raze__csv_core__0_1_10//:csv_core",
        "@raze__itoa__0_4_5//:itoa",
        "@raze__ryu__1_0_5//:ryu",
        "@raze__serde__1_0_111//:serde",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "1.1.3",
    crate_features = [
    ],
)

# Unsupported target "tests" with type "test" omitted
# Unsupported target "tutorial-error-01" with type "example" omitted
# Unsupported target "tutorial-error-02" with type "example" omitted
# Unsupported target "tutorial-error-03" with type "example" omitted
# Unsupported target "tutorial-error-04" with type "example" omitted
# Unsupported target "tutorial-perf-alloc-01" with type "example" omitted
# Unsupported target "tutorial-perf-alloc-02" with type "example" omitted
# Unsupported target "tutorial-perf-alloc-03" with type "example" omitted
# Unsupported target "tutorial-perf-core-01" with type "example" omitted
# Unsupported target "tutorial-perf-serde-01" with type "example" omitted
# Unsupported target "tutorial-perf-serde-02" with type "example" omitted
# Unsupported target "tutorial-perf-serde-03" with type "example" omitted
# Unsupported target "tutorial-pipeline-pop-01" with type "example" omitted
# Unsupported target "tutorial-pipeline-search-01" with type "example" omitted
# Unsupported target "tutorial-pipeline-search-02" with type "example" omitted
# Unsupported target "tutorial-read-01" with type "example" omitted
# Unsupported target "tutorial-read-delimiter-01" with type "example" omitted
# Unsupported target "tutorial-read-headers-01" with type "example" omitted
# Unsupported target "tutorial-read-headers-02" with type "example" omitted
# Unsupported target "tutorial-read-serde-01" with type "example" omitted
# Unsupported target "tutorial-read-serde-02" with type "example" omitted
# Unsupported target "tutorial-read-serde-03" with type "example" omitted
# Unsupported target "tutorial-read-serde-04" with type "example" omitted
# Unsupported target "tutorial-read-serde-invalid-01" with type "example" omitted
# Unsupported target "tutorial-read-serde-invalid-02" with type "example" omitted
# Unsupported target "tutorial-setup-01" with type "example" omitted
# Unsupported target "tutorial-write-01" with type "example" omitted
# Unsupported target "tutorial-write-02" with type "example" omitted
# Unsupported target "tutorial-write-delimiter-01" with type "example" omitted
# Unsupported target "tutorial-write-serde-01" with type "example" omitted
# Unsupported target "tutorial-write-serde-02" with type "example" omitted
