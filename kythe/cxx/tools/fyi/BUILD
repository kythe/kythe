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
        "//external:gflags",
        "//external:glog",
        "//external:proto/protobuf",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:net_client",
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
    ],
)

cc_binary(
    name = "fyi",
    deps = [
        ":fyilib",
    ],
)
