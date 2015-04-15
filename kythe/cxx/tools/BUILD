package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cmdlib",
    srcs = [
        "kindex_tool_main.cc",
    ],
    deps = [
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/proto:protobuf",
        "//third_party/zlib:zlib",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
)
cc_binary(
    name = "kindex_tool",
    deps = [
        ":cmdlib",
    ],
)
