package(default_visibility = ["//visibility:public"])

# Fork of https://github.com/bazelbuild/bazel-toolchains/blob/9a111bd8/configs/ubuntu16_04_clang/1.1/BUILD#L105
platform(
    name = "rbe",
    constraint_values = [
        "@bazel_tools//platforms:x86_64",
        "@bazel_tools//platforms:linux",
        "@bazel_tools//tools/cpp:clang",
        "@bazel_toolchains//constraints:xenial",
        "@bazel_toolchains//constraints/sanitizers:support_msan",
    ],
    remote_execution_properties = """
        properties: {
          name: "container-image"
          value:"docker://gcr.io/kythe-repo/kythe-builder@sha256:05bc48137d4effa7c29f7559b2f5c6abf7275674a9babe060835d473c25d1d2c"
        }
        """,
)
