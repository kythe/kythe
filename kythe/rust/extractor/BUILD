package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")
load(":extractor_test.bzl", "rust_extractor_test")

# These are used by the rust_extractor_test rule
alias(
    name = "rust_lib",
    actual = select({
        "//:darwin": "@rust_darwin_x86_64//:rustc_lib",
        "//conditions:default": "@rust_linux_x86_64//:rustc_lib",
    }),
)

alias(
    name = "rust_sysroot",
    actual = select({
        "//:darwin": "@rust_darwin_x86_64//:rust_lib-x86_64-apple-darwin",
        "//conditions:default": "@rust_linux_x86_64//:rust_lib-x86_64-unknown-linux-gnu",
    }),
)

rust_library(
    name = "kythe_rust_extractor",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**"],
    ),
    edition = "2018",
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
        ":extractor",
        ":extractor_integration_test_binary",
        ":kythe_rust_extractor",
    ],
)

rust_binary(
    name = "extractor_test_binary",
    srcs = glob(["tests/test.rs"]),
    edition = "2018",
    rustc_flags = [
        "--test",
    ],
    deps = [
        ":kythe_rust_extractor",
        "//kythe/rust/extractor/cargo:tempdir",
    ],
)

rust_extractor_test(
    name = "extractor_test",
    src = ":extractor_test_binary",
)

rust_binary(
    name = "extractor_integration_test_binary",
    srcs = glob(["tests/integration_test.rs"]),
    edition = "2018",
    deps = [
        "//kythe/proto:analysis_rust_proto",
        "//kythe/rust/extractor/cargo:anyhow",
        "//kythe/rust/extractor/cargo:assert_cmd",
        "//kythe/rust/extractor/cargo:predicates",
        "//kythe/rust/extractor/cargo:protobuf",
        "//kythe/rust/extractor/cargo:tempdir",
        "//kythe/rust/extractor/cargo:zip",
        "//third_party/bazel:extra_actions_base_rust_proto",
    ],
)

rust_extractor_test(
    name = "extractor_integration_test",
    src = ":extractor_integration_test_binary",
    tags = ["no-cache"],
)
