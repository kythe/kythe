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

load(
    "@io_bazel_rules_rust//cargo:cargo_build_script.bzl",
    "cargo_build_script",
)

cargo_build_script(
    name = "lexical_core_build_script",
    srcs = glob(["**/*.rs"]),
    crate_root = "build.rs",
    edition = "2015",
    deps = [
        "@raze__rustc_version__0_2_3//:rustc_version",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    crate_features = [
      "arrayvec",
      "correct",
      "default",
      "ryu",
      "static_assertions",
      "std",
      "table",
    ],
    data = glob(["**"]),
    version = "0.6.7",
    visibility = ["//visibility:private"],
)


rust_library(
    name = "lexical_core",
    crate_type = "lib",
    deps = [
        ":lexical_core_build_script",
        "@raze__arrayvec__0_4_12//:arrayvec",
        "@raze__bitflags__1_2_1//:bitflags",
        "@raze__cfg_if__0_1_9//:cfg_if",
        "@raze__ryu__1_0_5//:ryu",
        "@raze__static_assertions__0_3_4//:static_assertions",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2015",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.6.7",
    crate_features = [
        "arrayvec",
        "correct",
        "default",
        "ryu",
        "static_assertions",
        "std",
        "table",
    ],
)

