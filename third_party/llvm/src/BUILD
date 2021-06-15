licenses(["notice"])

package(default_visibility = ["//visibility:public"])

load("@io_kythe//tools:build_rules/cc_resources.bzl", "cc_resources")

cc_library(
    name = "preprocessor_util_lib",
    srcs = ["cxx_extractor_preprocessor_utils.cc"],
    hdrs = ["cxx_extractor_preprocessor_utils.h"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = ["@llvm-project//clang:lex"],
)

cc_resources(
    name = "clang_builtin_headers_resources",
    data = ["@llvm-project//clang:builtin_headers_gen"],
    strip = "staging/include/",
)

cc_library(
    name = "clang_builtin_headers",
    srcs = ["clang_builtin_headers.cc"],
    hdrs = ["clang_builtin_headers.h"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [":clang_builtin_headers_resources"],
)
