load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_clippy")

package(default_visibility = ["//visibility:public"])

rust_binary(
    name = "extra_action",
    srcs = glob(["src/**/*.rs"]),
    edition = "2021",
    deps = [
        "//third_party/bazel:extra_actions_base_rust_proto",
        "@crate_index//:anyhow",
        "@crate_index//:clap",
        "@rules_rust//proto/raze:protobuf",
    ],
)

rust_clippy(
    name = "clippy",
    deps = [
        ":extra_action",
    ],
)
