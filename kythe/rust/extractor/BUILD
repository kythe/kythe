package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library")
load(":extractor_test.bzl", "rust_extractor_test")

rust_library(
    name = "kythe_rust_extractor",
    srcs = glob(
        include = ["src/**/*.rs"],
        exclude = ["src/bin/**"],
    ),
    edition = "2018",
)

rust_clippy(
    name = "clippy",
    deps = [
        ":kythe_rust_extractor",
    ],
)

rust_binary(
    name = "extractor_test_binary",
    srcs = glob(["tests/**/*.rs"]),
    edition = "2018",
    deps = [
        ":kythe_rust_extractor",
        "//kythe/rust/extractor/cargo:tempdir",
    ],
    rustc_flags = [
        "--test",
    ],
)

rust_extractor_test(
    name = "extractor_test",
    src = ":extractor_test_binary",
    lib = select({
        "//:darwin": "@rust_darwin_x86_64//:rustc_lib",
        "//conditions:default": "@rust_linux_x86_64//:rustc_lib"
    }),
    sysroot = select({
        "//:darwin": "@rust_darwin_x86_64//:rust_lib-x86_64-apple-darwin",
        "//conditions:default": "@rust_linux_x86_64//:rust_lib-x86_64-unknown-linux-gnu",
    }),
)
