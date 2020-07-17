package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")

rust_library(
    name = "kythe_rust_extractor",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**"],
    ),
    edition = "2018",
    deps = [
        "//kythe/proto:analysis_rust_proto",
        "//kythe/rust/extractor/cargo:protobuf",
        "//kythe/rust/extractor/cargo:rls_data",
    ] + PROTO_COMPILE_DEPS,
)

rust_binary(
    name = "extractor",
    srcs = glob(["src/bin/**/*.rs"]),
    edition = "2018",
    deps = [
        ":kythe_rust_extractor",
        "//third_party/bazel:extra_actions_base_rust_proto",
        "//kythe/proto:analysis_rust_proto",
        "//kythe/rust/extractor/cargo:protobuf",
        "//kythe/rust/extractor/cargo:zip",
        "//kythe/rust/extractor/cargo:rls_data",
        "//kythe/rust/extractor/cargo:clap",
        "//kythe/rust/extractor/cargo:anyhow",
        "//kythe/rust/extractor/cargo:tempdir",
        "//kythe/rust/extractor/cargo:rust_crypto",
    ] + PROTO_COMPILE_DEPS,
)

rust_clippy(
    name = "clippy",
    deps = [
        ":kythe_rust_extractor",
        ":extractor",
    ],
)
