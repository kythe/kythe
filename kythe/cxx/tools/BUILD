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
        "//external:gflags",
        "//external:glog",
        "//external:proto/protobuf",
        "//external:zlib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:filecontext_proto_cc",
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
        "//external:gflags",
        "//external:glog",
        "//external:proto/protobuf",
        "//external:zlib",
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:claim_proto_cc",
    ],
)

cc_library(
    name = "shucklib",
    srcs = [
        "shuck_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:proto/protobuf",
        "//external:zlib",
        "//kythe/cxx/common:index_pack",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:claim_proto_cc",
    ],
)

cc_binary(
    name = "kindex_tool",
    deps = [
        ":cmdlib",
    ],
)

cc_binary(
    name = "shuck",
    deps = [
        ":shucklib",
    ],
)

cc_binary(
    name = "static_claim",
    deps = [
        ":claimcmdlib",
    ],
)

sh_binary(
    name = "kindex_file_generator",
    srcs = [
        "kindex_file_generator.sh",
    ],
    data = [
        ":kindex_tool",
    ],
)

sh_test(
    name = "kindex_file_generator_test",
    srcs = [
        "kindex_file_generator_test.sh",
    ],
    data = [
        ":kindex_file_generator",
        ":kindex_tool",
        "//kythe/testdata:BUILD",
    ],
)
