package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_clippy", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")

rust_library(
    name = "kythe_rust_indexer",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**"],
    ),
    edition = "2018",
    deps = [
        "//kythe/proto:analysis_rust_proto",
        "//kythe/proto:storage_rust_proto",
        "//kythe/rust/cargo:protobuf",
        "//kythe/rust/cargo:quick_error",
        "//kythe/rust/cargo:zip",
    ] + PROTO_COMPILE_DEPS,
)

rust_test(
    name = "inline_tests",
    crate = ":kythe_rust_indexer",
)

rust_test(
    name = "integration_tests",
    srcs = glob(["tests/*.rs"]),
    data = glob(["testfiles/**"]),
    deps = [
        ":kythe_rust_indexer",
        ":inline_tests",
        "//kythe/proto:analysis_rust_proto",
        "//kythe/proto:storage_rust_proto",
        "@io_bazel_rules_rust//tools/runfiles",
    ] + PROTO_COMPILE_DEPS,
)

rust_clippy(
    name = "clippy",
    deps = [
        ":kythe_rust_indexer",
    ],
)
