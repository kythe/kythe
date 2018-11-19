licenses(["notice"])

load("//tools:build_rules/cc_resources.bzl", "gendir")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "preprocessor_util_lib",
    srcs = [
        "cxx_extractor_preprocessor_utils.cc",
    ],
    hdrs = [
        "cxx_extractor_preprocessor_utils.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:zlib",
        "//kythe/cxx/extractor:cxx_details",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//third_party/llvm",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "clang_builtin_headers",
    srcs = [
        "clang_builtin_headers.cc",
        "//third_party/llvm:clang_builtin_headers_resources",
    ],
    hdrs = [
        "clang_builtin_headers.h",
        "//third_party/llvm:clang_builtin_headers_resources.inc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
        "-I" + gendir() + "/third_party/llvm",
    ],
)
