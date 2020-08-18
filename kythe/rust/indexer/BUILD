package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy", "rust_library", "rust_test")
load("@io_bazel_rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")
load("//tools/build_rules/verifier_test:rust_indexer_test.bzl", "rust_indexer_test")

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
        "//kythe/rust/cargo:rls_analysis",
        "//kythe/rust/cargo:rls_data",
    ] + PROTO_COMPILE_DEPS,
)

rust_binary(
    name = "indexer",
    srcs = glob(
        include = ["src/bin/**/*.rs"],
    ),
    crate_root = ":src/bin/main.rs",
    edition = "2018",
    deps = [
        ":kythe_rust_indexer",
        "//kythe/proto:analysis_rust_proto",
        "//kythe/proto:storage_rust_proto",
        "//kythe/rust/cargo:anyhow",
        "//kythe/rust/cargo:protobuf",
        "//kythe/rust/cargo:tempdir",
    ],
)

rust_test(
    name = "inline_tests",
    crate = ":kythe_rust_indexer",
)

rust_test(
    name = "integration_tests",
    srcs = glob(["tests/*.rs"]),
    data = [
        "tests/testanalysis.json",
        "tests/testkzip.kzip",
    ],
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
        ":indexer",
        ":kythe_rust_indexer",
    ],
)

rust_indexer_test(
    name = "anchors_test",
    srcs = ["testdata/anchors.rs"],
)

rust_indexer_test(
    name = "comment_test",
    srcs = ["testdata/comment.rs"],
)

rust_indexer_test(
    name = "enum_test",
    srcs = ["testdata/enum.rs"],
)

rust_indexer_test(
    name = "function_test",
    srcs = ["testdata/function.rs"],
)

rust_indexer_test(
    name = "modules_test",
    srcs = glob(["testdata/modules/*"]),
)

rust_indexer_test(
    name = "struct_test",
    srcs = ["testdata/struct.rs"],
)

rust_indexer_test(
    name = "tbuiltin_test",
    srcs = ["testdata/tbuiltin.rs"],
)

rust_indexer_test(
    name = "trait_test",
    srcs = ["testdata/trait.rs"],
)

rust_indexer_test(
    name = "type_test",
    srcs = ["testdata/type.rs"],
)

rust_indexer_test(
    name = "union_test",
    srcs = ["testdata/union.rs"],
)

rust_indexer_test(
    name = "variable_test",
    srcs = ["testdata/variable.rs"],
)

rust_indexer_test(
    name = "xrefs_test",
    srcs = glob(["testdata/xrefs/*.rs"]),
)
