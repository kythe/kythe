package(default_visibility = ["//visibility:public"])

# Fork of https://github.com/bazelbuild/bazel-toolchains/blob/89cf3b0d/configs/ubuntu16_04_clang/1.2/BUILD#L127
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
          value:"docker://gcr.io/kythe-repo/kythe-builder@sha256:9c246bf536f38ddd1fc9b492a4677b099127f0600cb1807ca6c96c6aa31a4f53"
        }
        """,
)
