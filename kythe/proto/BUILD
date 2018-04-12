package(default_visibility = ["//kythe:proto_visibility"])

load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")
load(":go.bzl", "go_kythe_proto")

# TODO(jrtom): add explore.proto to this once it's ready
filegroup(
    name = "public",
    srcs = [
        "analysis.proto",
        "analysis_service.proto",
        "buildinfo.proto",
        "common.proto",
        "cxx.proto",
        "extraction_config.proto",
        "filecontext.proto",
        "filetree.proto",
        "go.proto",
        "graph.proto",
        "identifier.proto",
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
)

cc_proto_library(
    name = "common_cc_proto",
    deps = [":common_proto"],
)

go_kythe_proto(proto = ":common_proto")

java_proto_library(
    name = "common_java_proto",
    deps = [":common_proto"],
)

# Public Kythe GraphStore analyzer output format
proto_library(
    name = "storage_proto",
    srcs = ["storage.proto"],
    visibility = [
        "//kythe:proto_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
)

cc_proto_library(
    name = "storage_cc_proto",
    deps = [":storage_proto"],
)

go_kythe_proto(proto = ":storage_proto")

java_proto_library(
    name = "storage_java_proto",
    deps = [":storage_proto"],
)

# Public Kythe GraphStore service API
proto_library(
    name = "storage_service_proto",
    srcs = ["storage_service.proto"],
    visibility = [
        "//kythe:proto_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [":storage_proto"],
)

cc_proto_library(
    name = "storage_service_cc_proto",
    deps = [":storage_service_proto"],
)

go_kythe_proto(
    proto = ":storage_service_proto",
    deps = [":storage_go_proto"],
)

java_proto_library(
    name = "storage_service_java_proto",
    deps = [":storage_service_proto"],
)

# Public Kythe analyzer
proto_library(
    name = "analysis_proto",
    srcs = ["analysis.proto"],
    visibility = [
        "//kythe:proto_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":filecontext_proto",
        ":storage_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "analysis_cc_proto",
    deps = [":analysis_proto"],
)

go_kythe_proto(
    proto = ":analysis_proto",
    deps = [
        ":filecontext_go_proto",
        ":storage_go_proto",
        "@go_protobuf//:ptypes/any",
    ],
)

java_proto_library(
    name = "analysis_java_proto",
    deps = [":analysis_proto"],
)

# Public Kythe file data service APIs
proto_library(
    name = "analysis_service_proto",
    srcs = ["analysis_service.proto"],
    visibility = [
        "//kythe:proto_visibility",
        "//third_party/llvm/src:__pkg__",
    ],
    deps = [
        ":analysis_proto",
        ":filecontext_proto",
        ":storage_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "analysis_service_cc_proto",
    deps = [":analysis_service_proto"],
)

go_kythe_proto(
    proto = ":analysis_service_proto",
    deps = [
        ":analysis_go_proto",
        ":filecontext_go_proto",
        ":storage_go_proto",
        "@go_protobuf//:ptypes/any",
    ],
)

java_proto_library(
    name = "analysis_service_java_proto",
    deps = [":analysis_service_proto"],
)

# Public Kythe status service API
proto_library(
    name = "status_service_proto",
    srcs = ["status_service.proto"],
)

cc_proto_library(
    name = "status_service_cc_proto",
    deps = [":status_service_proto"],
)

go_kythe_proto(proto = ":status_service_proto")

java_proto_library(
    name = "status_service_java_proto",
    deps = [":status_service_proto"],
)

# Context-dependent version information, for claiming.
proto_library(
    name = "filecontext_proto",
    srcs = ["filecontext.proto"],
)

cc_proto_library(
    name = "filecontext_cc_proto",
    deps = [":filecontext_proto"],
)

go_kythe_proto(proto = ":filecontext_proto")

java_proto_library(
    name = "filecontext_java_proto",
    deps = [":filecontext_proto"],
)

# Build tool details for compilation units
proto_library(
    name = "buildinfo_proto",
    srcs = ["buildinfo.proto"],
)

cc_proto_library(
    name = "buildinfo_cc_proto",
    deps = [":buildinfo_proto"],
)

go_kythe_proto(proto = ":buildinfo_proto")

java_proto_library(
    name = "buildinfo_java_proto",
    deps = [":buildinfo_proto"],
)

# C++-specific protocol buffer definitions
proto_library(
    name = "cxx_proto",
    srcs = ["cxx.proto"],
)

cc_proto_library(
    name = "cxx_cc_proto",
    deps = [":cxx_proto"],
)

go_kythe_proto(proto = ":cxx_proto")

java_proto_library(
    name = "cxx_java_proto",
    deps = [":cxx_proto"],
)

# Go-specific protocol buffer definitions
proto_library(
    name = "go_proto",
    srcs = ["go.proto"],
)

cc_proto_library(
    name = "go_cc_proto",
    deps = [":go_proto"],
)

go_kythe_proto(proto = ":go_proto")

java_proto_library(
    name = "go_java_proto",
    deps = [":go_proto"],
)

# Java-specific protocol buffer definitions
proto_library(
    name = "java_proto",
    srcs = ["java.proto"],
    deps = [
        ":analysis_proto",
        ":storage_proto",
    ],
)

cc_proto_library(
    name = "java_cc_proto",
    deps = [":java_proto"],
)

go_kythe_proto(
    proto = ":java_proto",
    deps = [
        ":analysis_go_proto",
        ":storage_go_proto",
    ],
)

java_proto_library(
    name = "java_java_proto",
    deps = [":java_proto"],
)

# Kythe claiming message definitions
proto_library(
    name = "claim_proto",
    srcs = ["claim.proto"],
    deps = [":storage_proto"],
)

cc_proto_library(
    name = "claim_cc_proto",
    deps = [":claim_proto"],
)

go_kythe_proto(
    proto = ":claim_proto",
    deps = [":storage_go_proto"],
)

java_proto_library(
    name = "claim_java_proto",
    deps = [":claim_proto"],
)

# Public Kythe filetree service API
proto_library(
    name = "filetree_proto",
    srcs = ["filetree.proto"],
)

cc_proto_library(
    name = "filetree_cc_proto",
    deps = [":filetree_proto"],
)

go_kythe_proto(proto = ":filetree_proto")

java_proto_library(
    name = "filetree_java_proto",
    deps = [":filetree_proto"],
)

# Public Kythe xref service API
proto_library(
    name = "xref_proto",
    srcs = ["xref.proto"],
    visibility = [
        # TODO(schroederc): prune once MarkedSource has migrated to common.proto
        "//kythe/cxx/common:__pkg__",
        "//kythe/cxx/doc:__pkg__",
        "//kythe/go/languageserver:__subpackages__",
        "//kythe/go/platform/tools:__subpackages__",
        "//kythe/go/services:__subpackages__",
        "//kythe/go/serving:__subpackages__",
        "//kythe/go/test:__subpackages__",
        "//kythe/go/util/markedsource:__pkg__",
        "//kythe/java/com/google/devtools/kythe/doc:__subpackages__",
        "//kythe/javatests/com/google/devtools/kythe/doc:__subpackages__",
    ],
    deps = [":common_proto"],
)

cc_proto_library(
    name = "xref_cc_proto",
    deps = [":xref_proto"],
)

go_kythe_proto(
    proto = ":xref_proto",
    deps = [":common_go_proto"],
)

java_proto_library(
    name = "xref_java_proto",
    deps = [":xref_proto"],
)

proto_library(
    name = "identifier_proto",
    srcs = ["identifier.proto"],
)

cc_proto_library(
    name = "identifier_cc_proto",
    deps = [":identifier_proto"],
)

go_kythe_proto(proto = ":identifier_proto")

java_proto_library(
    name = "identifier_java_proto",
    deps = [":identifier_proto"],
)

# Public Kythe graph service API
proto_library(
    name = "graph_proto",
    srcs = ["graph.proto"],
    deps = [":common_proto"],
)

go_kythe_proto(
    proto = ":graph_proto",
    deps = [":common_go_proto"],
)

cc_proto_library(
    name = "graph_cc_proto",
    deps = [":graph_proto"],
)

java_proto_library(
    name = "graph_java_proto",
    deps = [":graph_proto"],
)

# Public Kythe graph exploration API
proto_library(
    name = "explore_proto",
    srcs = ["explore.proto"],
    deps = [
        ":common_proto",
        ":storage_proto",
        ":xref_proto",
    ],
)

go_kythe_proto(
    proto = ":explore_proto",
    deps = [
        ":common_go_proto",
        ":storage_go_proto",
        ":xref_go_proto",
    ],
)

cc_proto_library(
    name = "explore_cc_proto",
    deps = [":explore_proto"],
)

java_proto_library(
    name = "explore_java_proto",
    deps = [":explore_proto"],
)

# Protocol buffer definitions used to encode serving data
# WARNING: These should not be exposed to clients.
proto_library(
    name = "serving_proto",
    srcs = ["serving.proto"],
    visibility = [
        "//kythe/go/serving:__subpackages__",
        "//kythe/go/util/tools:__subpackages__",
        "//kythe/java/com/google/devtools/kythe/util:__subpackages__",
        "//kythe/javatests/com/google/devtools/kythe/util:__subpackages__",
    ],
    deps = [":common_proto"],
)

cc_proto_library(
    name = "serving_cc_proto",
    deps = [":serving_proto"],
)

go_kythe_proto(
    proto = ":serving_proto",
    deps = [":common_go_proto"],
)

java_proto_library(
    name = "serving_java_proto",
    deps = [":serving_proto"],
)

proto_library(
    name = "link_proto",
    srcs = ["link.proto"],
    deps = [":common_proto"],
)

cc_proto_library(
    name = "link_cc_proto",
    deps = [":link_proto"],
)

go_kythe_proto(
    proto = ":link_proto",
    deps = [
        ":common_go_proto",
        ":identifier_go_proto",
        ":xref_go_proto",
    ],
)

java_proto_library(
    name = "link_java_proto",
    deps = [":link_proto"],
)

# Protocol buffer definitions internal only to the Kythe libraries and tools.
# WARNING: These should not be exposed to clients.
proto_library(
    name = "internal_proto",
    srcs = ["internal.proto"],
    deps = [
        ":common_proto",
        ":serving_proto",
    ],
)

cc_proto_library(
    name = "internal_cc_proto",
    deps = [":internal_proto"],
)

go_kythe_proto(
    proto = ":internal_proto",
    deps = [
        ":common_go_proto",
        ":serving_go_proto",
    ],
)

java_proto_library(
    name = "internal_java_proto",
    deps = [":internal_proto"],
)

# Protocol buffer definitions used to record indexer outputs for storage.
# WARNING: These should not be exposed to clients.
proto_library(
    name = "entryset_proto",
    srcs = ["entryset.proto"],
)

cc_proto_library(
    name = "entryset_cc_proto",
    deps = [":entryset_proto"],
)

go_kythe_proto(proto = ":entryset_proto")

java_proto_library(
    name = "entryset_java_proto",
    deps = [":entryset_proto"],
)

proto_library(
    name = "extraction_config_proto",
    srcs = ["extraction_config.proto"],
    visibility = [
        "//kythe:proto_visibility",
    ],
)

go_proto_library(
    name = "extraction_config_go_proto",
    proto = ":extraction_config_proto",
    importpath = "kythe.io/kythe/proto/extraction_config_go_proto",
)
