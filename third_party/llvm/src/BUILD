licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "preprocessor_util_lib",
    srcs = ["cxx_extractor_preprocessor_utils.cc"],
    hdrs = ["cxx_extractor_preprocessor_utils.h"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = ["@org_llvm//:clangLex"],
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
    deps = ["@org_llvm//:clang_builtin_headers_resources"],
)
