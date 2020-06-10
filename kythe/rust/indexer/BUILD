package(default_visibility = ["//visibility:public"])
load("@io_bazel_rules_rust//rust:rust.bzl", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")

rust_library(
  name = "kythe_rust_indexer",
  srcs = glob(
    include = ["src/**/*.rs"],
    exclude = ["src/bin/**"]
  ),
  deps = [
    "//kythe/proto:analysis_rust_proto",
    "//kythe/rust/indexer/cargo:positioned_io",
    "//kythe/rust/indexer/cargo:protobuf",
    "//kythe/rust/indexer/cargo:rc_zip",
  ] + PROTO_COMPILE_DEPS,
  edition = "2018",
)

rust_test(
  name = "inline_tests",
  crate = ":kythe_rust_indexer",
)

rust_test(
  name = "integration_tests",
  srcs = glob(["tests/*.rs"]),
  deps = [":kythe_rust_indexer", ":inline_tests"],
  data = glob(["testfiles/**"]),
)
