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
  "notice", # MIT from expression "MIT"
])

load(
    "@io_bazel_rules_rust//rust:rust.bzl",
    "rust_library",
    "rust_binary",
    "rust_test",
)


# Unsupported target "end_to_end" with type "test" omitted
# Unsupported target "extract" with type "example" omitted
# Unsupported target "extract_lorem" with type "example" omitted
# Unsupported target "file_info" with type "example" omitted
# Unsupported target "invalid_date" with type "test" omitted
# Unsupported target "read_entry" with type "bench" omitted
# Unsupported target "stdin_info" with type "example" omitted
# Unsupported target "write_dir" with type "example" omitted
# Unsupported target "write_sample" with type "example" omitted

rust_library(
    name = "zip",
    crate_type = "lib",
    deps = [
        "@raze__bzip2__0_3_3//:bzip2",
        "@raze__crc32fast__1_2_0//:crc32fast",
        "@raze__flate2__1_0_14//:flate2",
        "@raze__podio__0_1_7//:podio",
        "@raze__time__0_1_43//:time",
    ],
    srcs = glob(["**/*.rs"]),
    crate_root = "src/lib.rs",
    edition = "2018",
    rustc_flags = [
        "--cap-lints=allow",
    ],
    version = "0.5.6",
    tags = ["cargo-raze"],
    crate_features = [
        "bzip2",
        "default",
        "deflate",
        "flate2",
        "time",
    ],
)

# Unsupported target "zip64_large" with type "test" omitted
