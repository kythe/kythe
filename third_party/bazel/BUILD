load("/tools/build_rules/genproto", "proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

proto_library(
    name = "extra_actions_base_proto",
    src = "src/main/protobuf/extra_actions_base.proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    go_package = "third_party/bazel/extra_actions_base_proto",
)

proto_library(
    name = "test_status_proto",
    src = "src/main/protobuf/test_status.proto",
    gen_go = 1,
    go_package = "third_party/bazel/test_status_proto",
)
