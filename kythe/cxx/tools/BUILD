package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "cmdlib",
    srcs = [
        "kindex_tool_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
    ],
)

cc_library(
    name = "claimcmdlib",
    srcs = [
        "static_claim_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:claim_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
    ],
)

cc_binary(
    name = "kindex_tool",
    deps = [
        ":cmdlib",
    ],
)

cc_binary(
    name = "static_claim",
    deps = [
        ":claimcmdlib",
    ],
)
