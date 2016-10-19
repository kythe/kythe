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
        "//third_party/llvm",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/proto:protobuf",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "fyi",
    deps = [
        ":fyilib",
    ],
)
