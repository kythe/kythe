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



rust_library(
    name = "rc_zip",
    crate_root = "src/lib.rs",
    crate_type = "lib",
    edition = "2018",
    srcs = glob(["**/*.rs"]),
    deps = [
        "@raze__chardet__0_2_4//:chardet",
        "@raze__chrono__0_4_11//:chrono",
        "@raze__circular__0_3_0//:circular",
        "@raze__codepage_437__0_1_0//:codepage_437",
        "@raze__crc32fast__1_2_0//:crc32fast",
        "@raze__encoding_rs__0_8_23//:encoding_rs",
        "@raze__hex_fmt__0_3_0//:hex_fmt",
        "@raze__libflate__0_1_27//:libflate",
        "@raze__log__0_4_8//:log",
        "@raze__nom__5_1_1//:nom",
        "@raze__positioned_io__0_2_2//:positioned_io",
        "@raze__pretty_hex__0_1_1//:pretty_hex",
    ],
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.0.1",
    crate_features = [
    ],
)

