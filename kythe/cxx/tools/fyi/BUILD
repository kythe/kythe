package(default_visibility = ["//kythe:default_visibility"])

cc_library(
    name = "fyilib",
    srcs = [
        "fyi.cc",
        "fyi_main.cc",
    ],
    hdrs = [
        "fyi.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:net_client",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/proto:protobuf",
    ],
)

cc_binary(
    name = "fyi",
    deps = [
        ":fyilib",
    ],
)
