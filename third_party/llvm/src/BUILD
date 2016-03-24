licenses(["notice"])

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
        "//external:gflags",
        "//external:re2",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//external:glog",
        "//third_party/llvm",
        "//third_party/proto:protobuf",
        "//third_party/zlib",
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
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
        "-I$(GENDIR)/third_party/llvm",
    ],
)
