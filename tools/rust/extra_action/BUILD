load("@rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy")

package(default_visibility = ["//visibility:public"])

rust_binary(
    name = "extra_action",
    srcs = glob(["src/**/*.rs"]),
    edition = "2018",
    deps = [
        "//kythe/rust/cargo:anyhow",
        "//kythe/rust/cargo:clap",
        "//third_party/bazel:extra_actions_base_rust_proto",
        "@rules_rust//proto/raze:protobuf",
    ],
)

rust_clippy(
    name = "clippy",
    deps = [
        ":extra_action",
    ],
)
