package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/proto.bzl", "proto_library")

proto_library(
    name = "example_proto",
    srcs = ["example.proto"],
    cc_api_version = 2,
    emit_metadata = True,
)

cc_library(
    name = "proto_user",
    srcs = ["proto_user.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    visibility = [
        "//kythe:default_visibility",
    ],
    deps = [
        ":example_proto_cc",
    ],
)

cc_binary(
    name = "proto_indexer",
    srcs = ["proto_indexer.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    visibility = [
        "//kythe:default_visibility",
    ],
    deps = [
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common/indexing:lib",
        "//third_party/proto:cc_wkt_protos",
        "@com_github_google_glog//:glog",
    ],
)
