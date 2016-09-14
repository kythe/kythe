package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/proto.bzl", "proto_library")

filegroup(
    name = "public",
    srcs = [
        "analysis.proto",
        "analysis_service.proto",
        "buildinfo.proto",
        "common.proto",
        "cxx.proto",
        "filecontext.proto",
        "filetree.proto",
        "java.proto",
        "storage.proto",
        "storage_service.proto",
        "xref.proto",
    ],
)

# Common Kythe protocol buffer definitions shared among other libraries
proto_library(
    name = "common_proto",
    gen_cc = 1,
    gen_go = 1,
)

# Public Kythe GraphStore analyzer output format
proto_library(
    name = "storage_proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
)

# Public Kythe GraphStore service API
proto_library(
    name = "storage_service_proto",
    has_services = 1,
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [":storage_proto"],
)

# Public Kythe analyzer
proto_library(
    name = "analysis_proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":filecontext_proto",
        ":storage_proto",
        "//external:proto/any_proto",
    ],
)

# Public Kythe file data service APIs
proto_library(
    name = "analysis_service_proto",
    has_services = 1,
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":analysis_proto",
        ":filecontext_proto",
        ":storage_proto",
        "//external:proto/any_proto",
    ],
)

# Context-dependent version information, for claiming.
proto_library(
    name = "filecontext_proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
)

# Build tool details for compilation units
proto_library(
    name = "buildinfo_proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
)

# C++-specific protocol buffer definitions
proto_library(
    name = "cxx_proto",
    gen_cc = 1,
)

# Java-specific protocol buffer definitions
proto_library(
    name = "java_proto",
    gen_java = 1,
)

# Kythe claiming message definitions
proto_library(
    name = "claim_proto",
    gen_cc = 1,
    deps = [":storage_proto"],
)

# Public Kythe filetree service API
proto_library(
    name = "filetree_proto",
    has_services = 1,
    gen_go = 1,
)

# Public Kythe xref service API
proto_library(
    name = "xref_proto",
    has_services = 1,
    gen_cc = 1,
    gen_go = 1,
)

# Protocol buffer definitions used to encode serving data
# WARNING: These should not be exposed to clients.
proto_library(
    name = "serving_proto",
    gen_cc = 1,
    gen_go = 1,
    deps = [":common_proto"],
)

# Protocol buffer definitions internal only to the Kythe libraries and tools.
# WARNING: These should not be exposed to clients.
proto_library(
    name = "internal_proto",
    gen_go = 1,
    deps = [
        ":common_proto",
        ":serving_proto",
    ],
)
