load("/tools/build_rules/proto", "proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

proto_library(
    name = "extra_actions_base_proto",
    src = "src/main/protobuf/extra_actions_base.proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
)

proto_library(
    name = "test_status_proto",
    src = "src/main/protobuf/test_status.proto",
    gen_go = 1,
)
