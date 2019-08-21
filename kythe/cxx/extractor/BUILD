package(default_visibility = ["//kythe:default_visibility"])

load("//:visibility.bzl", "PUBLIC_VISIBILITY")

cc_library(
    name = "index_pack",
    srcs = ["index_pack.cc"],
    hdrs = ["index_pack.h"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deprecation = "Please use :kzip_{reader,writer}",
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//external:libuuid",
        "//external:zlib",
        "//kythe/cxx/common:lib",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "@boringssl//:crypto",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
        "@org_llvm//:LLVMSupport",
    ],
)

cc_library(
    name = "index_pack_testlib",
    testonly = 1,
    srcs = ["index_pack_test.cc"],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":index_pack",
        "//external:zlib",
        "//third_party:gtest",
        "@com_github_google_glog//:glog",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "index_pack_test",
    size = "small",
    deps = [":index_pack_testlib"],
)

cc_library(
    name = "cxx_details",
    srcs = [
        "cxx_details.cc",
    ],
    hdrs = [
        "cxx_details.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//kythe/proto:cxx_cc_proto",
        "@com_github_google_glog//:glog",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangLex",
    ],
)

cc_library(
    name = "path_utils",
    srcs = ["path_utils.cc"],
    hdrs = ["path_utils.h"],
    deps = [
        "//kythe/cxx/common:lib",
        "@com_google_absl//absl/strings:str_format",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangBasic",
        "@org_llvm//:clangLex",
        "@org_llvm//:clangTooling",
    ],
)

cc_library(
    name = "supported_language",
    srcs = ["language.cc"],
    hdrs = ["language.h"],
    deps = ["@org_llvm//:LLVMSupport"],
)

cc_library(
    name = "command_line_utils",
    srcs = [
        "CommandLineUtils.cc",
    ],
    hdrs = [
        "CommandLineUtils.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/strings:str_format",
        "@org_llvm//:LLVMSupport",
    ],
)

cc_library(
    name = "commandline_testlib",
    testonly = 1,
    srcs = [
        "CommandLineUtilsTest.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":command_line_utils",
        "//third_party:gtest",
    ],
)

cc_test(
    name = "commandline_test",
    size = "small",
    deps = [
        ":commandline_testlib",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "cxx_extractor.cc",
    ],
    hdrs = [
        "cxx_extractor.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":command_line_utils",
        ":cxx_details",
        ":path_utils",
        ":supported_language",
        "//external:zlib",
        "//kythe/cxx/common:file_utils",
        "//kythe/cxx/common:index_writer",
        "//kythe/cxx/common:json_proto",
        "//kythe/cxx/common:kzip_writer",
        "//kythe/cxx/common:lib",
        "//kythe/cxx/common:path_utils",
        "//kythe/cxx/indexer/cxx:clang_utils",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:buildinfo_cc_proto",
        "//kythe/proto:cxx_cc_proto",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/llvm/src:preprocessor_util_lib",
        "@boringssl//:crypto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangTooling",
    ],
)

cc_library(
    name = "cmdlib",
    srcs = [
        "cxx_extractor_main.cc",
    ],
    hdrs = [
        "cxx_extractor.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        ":supported_language",
        "//external:zlib",
        "//kythe/proto:analysis_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
        "@org_llvm//:clangTooling",
    ],
)

cc_library(
    name = "bazelcmdlib",
    srcs = [
        "cxx_extractor_bazel_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        ":supported_language",
        "//external:zlib",
        "//third_party/bazel:extra_actions_base_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "objcbazelcmdlib",
    srcs = [
        "objc_extractor_bazel_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        ":objc_bazel_support_library",
        ":supported_language",
        "//external:zlib",
        "//kythe/cxx/common:path_utils",
        "//third_party/bazel:extra_actions_base_cc_proto",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "objc_bazel_support_library",
    srcs = ["objc_bazel_support.cc"],
    hdrs = ["objc_bazel_support.h"],
    deps = [
        "//third_party/bazel:extra_actions_base_cc_proto",
        "@com_googlesource_code_re2//:re2",
        "@org_llvm//:LLVMSupport",
    ],
)

cc_test(
    name = "objc_bazel_support_library_test",
    size = "small",
    srcs = ["objc_bazel_support_test.cc"],
    deps = [
        ":objc_bazel_support_library",
        "//third_party:gtest",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "cxx_extractor",
    deps = [
        ":cmdlib",
    ],
)

cc_binary(
    name = "cxx_extractor_bazel",
    visibility = ["//visibility:public"],
    deps = [
        ":bazelcmdlib",
    ],
)

cc_binary(
    name = "objc_extractor_bazel",
    visibility = ["//visibility:public"],
    deps = [
        ":objcbazelcmdlib",
    ],
)

cc_library(
    name = "testlib",
    testonly = 1,
    srcs = [
        "cxx_extractor_test.cc",
    ],
    hdrs = [
        "cxx_extractor.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//external:zlib",
        "//kythe/cxx/common:path_utils",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gtest",
        "@com_github_google_glog//:glog",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
        "@org_llvm//:LLVMSupport",
        "@org_llvm//:clangFrontend",
        "@org_llvm//:clangTooling",
    ],
)

cc_test(
    name = "test",
    size = "small",
    deps = [
        ":testlib",
    ],
)
