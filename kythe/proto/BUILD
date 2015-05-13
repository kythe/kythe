package(default_visibility = ["//kythe:default_visibility"])

load("/tools/build_rules/genproto", "proto_library")

proto_library(
    name = "any_proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
)

proto_library(
    name = "storage_proto",
    has_services = 1,
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
)

proto_library(
    name = "analysis_proto",
    has_services = 1,
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":any_proto",
        ":storage_proto",
    ],
)

proto_library(
    name = "cxx_proto",
    gen_cc = 1,
)

proto_library(
    name = "claim_proto",
    gen_cc = 1,
    deps = [":storage_proto"],
)

proto_library(
    name = "xref_proto",
    has_services = 1,
    gen_cc = 1,
    gen_go = 1,
)

proto_library(
    name = "serving_proto",
    has_services = 1,
    gen_go = 1,
)

proto_library(
    name = "filetree_proto",
    has_services = 1,
    gen_go = 1,
)
