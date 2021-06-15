load("@rules_rust//rust:rust.bzl", "rust_binary", "rust_clippy")
load("@rules_rust//proto:toolchain.bzl", "PROTO_COMPILE_DEPS")

package(default_visibility = ["//visibility:public"])

rust_binary(
    name = "extra_action",
    srcs = glob(["src/**/*.rs"]),
    edition = "2018",
    deps = [
        "//third_party/bazel:extra_actions_base_rust_proto",
        "//kythe/rust/cargo:clap",
        "//kythe/rust/cargo:anyhow",
    ] + PROTO_COMPILE_DEPS,
)

rust_clippy(
    name = "clippy",
    deps = [
        ":extra_action",
    ],
)
