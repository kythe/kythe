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
        "go.proto",
        "graph.proto",
        "java.proto",
        "status_service.proto",
        "storage.proto",
        "storage_service.proto",
        "xref.proto",
    ],
)

# Common Kythe protocol buffer definitions shared among other libraries
proto_library(
    name = "common_proto",
    srcs = ["common.proto"],
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
)

# Public Kythe GraphStore analyzer output format
proto_library(
    name = "storage_proto",
    srcs = ["storage.proto"],
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
)

# Public Kythe GraphStore service API
proto_library(
    name = "storage_service_proto",
    srcs = ["storage_service.proto"],
    has_services = 1,
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [":storage_proto"],
)

# Public Kythe analyzer
proto_library(
    name = "analysis_proto",
    srcs = ["analysis.proto"],
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":filecontext_proto",
        ":storage_proto",
        "//third_party/proto:any_proto",
    ],
)

# Public Kythe file data service APIs
proto_library(
    name = "analysis_service_proto",
    srcs = ["analysis_service.proto"],
    has_services = 1,
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
    visibility = [
        "//kythe:default_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":analysis_proto",
        ":filecontext_proto",
        ":storage_proto",
        "//third_party/proto:any_proto",
    ],
)

# Public Kythe status service API
proto_library(
    name = "status_service_proto",
    srcs = ["status_service.proto"],
    has_services = 1,
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
)

# Context-dependent version information, for claiming.
proto_library(
    name = "filecontext_proto",
    srcs = ["filecontext.proto"],
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
)

# Build tool details for compilation units
proto_library(
    name = "buildinfo_proto",
    srcs = ["buildinfo.proto"],
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
)

# C++-specific protocol buffer definitions
proto_library(
    name = "cxx_proto",
    srcs = ["cxx.proto"],
    cc_api_version = 2,
    go_api_version = 2,
)

# Go-specific protocol buffer definitions
proto_library(
    name = "go_proto",
    srcs = ["go.proto"],
    go_api_version = 2,
)

# Java-specific protocol buffer definitions
proto_library(
    name = "java_proto",
    srcs = ["java.proto"],
    go_api_version = 2,
    java_api_version = 2,
)

# Kythe claiming message definitions
proto_library(
    name = "claim_proto",
    srcs = ["claim.proto"],
    cc_api_version = 2,
    deps = [":storage_proto"],
)

# Public Kythe filetree service API
proto_library(
    name = "filetree_proto",
    srcs = ["filetree.proto"],
    has_services = 1,
    go_api_version = 2,
)

# Public Kythe xref service API
proto_library(
    name = "xref_proto",
    srcs = ["xref.proto"],
    has_services = 1,
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
    visibility = [
        # TODO(schroederc): prune once MarkedSource has migrated to common.proto
        "//kythe/cxx/common:__pkg__",
        "//kythe/cxx/doc:__pkg__",
        "//kythe/go/platform/tools:__subpackages__",
        "//kythe/go/services:__subpackages__",
        "//kythe/go/serving:__subpackages__",
        "//kythe/go/test:__subpackages__",
        "//kythe/go/util/markedsource:__pkg__",
    ],
    deps = [":common_proto"],
)

# Public Kythe graph service API
proto_library(
    name = "graph_proto",
    srcs = ["graph.proto"],
    has_services = 1,
    cc_api_version = 2,
    go_api_version = 2,
    deps = [":common_proto"],
)

# Protocol buffer definitions used to encode serving data
# WARNING: These should not be exposed to clients.
proto_library(
    name = "serving_proto",
    srcs = ["serving.proto"],
    cc_api_version = 2,
    go_api_version = 2,
    java_api_version = 2,
    visibility = [
        "//kythe/go/serving:__subpackages__",
        "//kythe/go/util/tools:__pkg__",
    ],
    deps = [":common_proto"],
)

# Protocol buffer definitions internal only to the Kythe libraries and tools.
# WARNING: These should not be exposed to clients.
proto_library(
    name = "internal_proto",
    srcs = ["internal.proto"],
    go_api_version = 2,
    java_api_version = 2,
    deps = [
        ":common_proto",
        ":serving_proto",
    ],
)

# Protocol buffer definitions used to record indexer outputs for storage.
# WARNING: These should not be exposed to clients.
proto_library(
    name = "entryset_proto",
    srcs = ["entryset.proto"],
    go_api_version = 2,
)
