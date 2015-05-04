load("//tools/build_rules/genproto", "genproto_all")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

genproto_all(
    name = "extra_actions_base_proto",
    src = "src/main/protobuf/extra_actions_base.proto",
    go_package_prefix = "",
)

genproto_all(
    name = "test_status_proto",
    src = "src/main/protobuf/test_status.proto",
    go_package_prefix = "",
)
