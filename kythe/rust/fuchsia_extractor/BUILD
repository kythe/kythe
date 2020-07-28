package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")

rust_library(
    name = "fuchsia_extractor_lib",
    srcs = glob(
        include = ["src/**/*.rs"],
    ),
    deps = [
      "//kythe/cxx/common:kzip_writer_c_api",
      "//kythe/proto:analysis_rust_proto",
      "//kythe/rust/extractor/cargo:anyhow",
      "//kythe/rust/extractor/cargo:protobuf",
    ],
    edition = "2018",
)

rust_test(
    name = "fuchsia_extractor_lib_test",
    crate = ":fuchsia_extractor_lib",
    deps = [
      "//kythe/rust/extractor/cargo:tempdir",
    ],
    edition = "2018",
)

rust_clippy(
    name = "clippy",
    deps = [
        ":fuchsia_extractor_lib",
    ],
)

