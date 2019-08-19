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
        "//kythe/cxx/common:kythe_uri",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:net_client",
        "//kythe/cxx/common/schema:edges",
        "//kythe/cxx/common/schema:facts",
        "//kythe/cxx/indexer/cxx:clang_utils",
        "//third_party/llvm/src:clang_builtin_headers",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangFrontend",
        "@org_llvm//:clangLex",
        "@org_llvm//:clangParse",
        "@org_llvm//:clangRewrite",
        "@org_llvm//:clangSema",
        "@org_llvm//:clangTooling",
    ],
)

cc_binary(
    name = "fyi",
    deps = [
        ":fyilib",
    ],
)
