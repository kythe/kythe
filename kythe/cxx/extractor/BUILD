load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load(":toolchain.bzl", "cxx_extractor_toolchain")

package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
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
        "@com_google_absl//absl/log",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:lex",
    ],
)

cc_library(
    name = "path_utils",
    srcs = ["path_utils.cc"],
    hdrs = ["path_utils.h"],
    deps = [
        "//kythe/cxx/common:lib",
        "@com_google_absl//absl/strings:str_format",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:lex",
        "@llvm-project//clang:tooling",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "supported_language",
    srcs = ["language.cc"],
    hdrs = ["language.h"],
    deps = ["@llvm-project//llvm:Support"],
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
        "@com_google_absl//absl/functional:function_ref",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings:str_format",
        "@llvm-project//llvm:Support",
    ],
)

cc_test(
    name = "commandline_test",
    size = "small",
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
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
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
        "//kythe/cxx/common:sha256_hasher",
        "//kythe/cxx/indexer/cxx:clang_utils",
        "//kythe/cxx/indexer/cxx:stream_adapter",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:buildinfo_cc_proto",
        "//kythe/proto:cxx_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "//third_party/llvm/src:clang_builtin_headers",
        "//third_party/llvm/src:preprocessor_util_lib",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io:gzip_stream",
        "@com_googlesource_code_re2//:re2",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:lex",
        "@llvm-project//clang:tooling",
        "@llvm-project//llvm:Support",
    ],
)

cc_binary(
    name = "cxx_extractor",
    srcs = ["cxx_extractor_main.cc"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":lib",
        ":supported_language",
        "//kythe/cxx/common:init",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "objc_bazel_support_library",
    srcs = ["objc_bazel_support.cc"],
    hdrs = ["objc_bazel_support.h"],
    deps = [
        "//third_party/bazel:extra_actions_base_cc_proto",
        "@com_googlesource_code_re2//:re2",
        "@llvm-project//llvm:Support",
    ],
)

cc_test(
    name = "objc_bazel_support_library_test",
    size = "small",
    srcs = ["objc_bazel_support_test.cc"],
    deps = [
        ":objc_bazel_support_library",
        "//third_party:gtest",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:initialize",
    ],
)

cc_binary(
    name = "cxx_extractor_bazel",
    srcs = [
        "cxx_extractor_bazel_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":lib",
        ":supported_language",
        "//external:zlib",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:path_utils",
        "//third_party/bazel:extra_actions_base_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_binary(
    name = "objc_extractor_bazel",
    srcs = [
        "objc_extractor_bazel_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":lib",
        ":objc_bazel_support_library",
        ":supported_language",
        "//external:zlib",
        "//kythe/cxx/common:init",
        "//kythe/cxx/common:path_utils",
        "//third_party/bazel:extra_actions_base_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "test",
    size = "small",
    srcs = [
        "cxx_extractor_test.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":lib",
        "//external:zlib",
        "//kythe/cxx/common:path_utils",
        "//kythe/proto:analysis_cc_proto",
        "//third_party:gtest",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:initialize",
        "@com_google_absl//absl/memory",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:tooling",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "testlib",
    testonly = 1,
    srcs = ["testlib.cc"],
    hdrs = ["testlib.h"],
    data = [":cxx_extractor"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//kythe/cxx/common:kzip_reader",
        "//kythe/cxx/common:path_utils",
        "//kythe/proto:analysis_cc_proto",
        "//kythe/proto:cxx_cc_proto",
        "//kythe/proto:filecontext_cc_proto",
        "//third_party:gtest",
        "@bazel_tools//tools/cpp/runfiles",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "bazel_event_reader",
    srcs = ["bazel_event_reader.cc"],
    hdrs = ["bazel_event_reader.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//third_party/bazel:build_event_stream_cc_proto",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:variant",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "bazel_event_reader_test",
    srcs = ["bazel_event_reader_test.cc"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":bazel_event_reader",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "//third_party/bazel:build_event_stream_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)

cc_library(
    name = "bazel_artifact",
    hdrs = ["bazel_artifact.h"],
    deps = [
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "bazel_artifact_selector",
    srcs = ["bazel_artifact_selector.cc"],
    hdrs = ["bazel_artifact_selector.h"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":bazel_artifact",
        "//kythe/cxx/common:regex",
        "//kythe/proto:bazel_artifact_selector_cc_proto",
        "//kythe/proto:bazel_artifact_selector_v2_cc_proto",
        "//third_party/bazel:build_event_stream_cc_proto",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/hash",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "bazel_artifact_selector_test",
    srcs = ["bazel_artifact_selector_test.cc"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":bazel_artifact",
        ":bazel_artifact_selector",
        "//kythe/cxx/common:regex",
        "//kythe/proto:bazel_artifact_selector_v2_cc_proto",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "//third_party/bazel:build_event_stream_cc_proto",
        "@com_github_inazarenko_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/functional:any_invocable",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "bazel_artifact_reader",
    srcs = ["bazel_artifact_reader.cc"],
    hdrs = ["bazel_artifact_reader.h"],
    deps = [
        ":bazel_artifact",
        ":bazel_artifact_selector",
        ":bazel_event_reader",
        "//third_party/bazel:build_event_stream_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "bazel_artifact_reader_test",
    srcs = ["bazel_artifact_reader_test.cc"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":bazel_artifact_reader",
        ":bazel_event_reader",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "//third_party/bazel:build_event_stream_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)

cc_binary(
    name = "dump_bazel_artifacts",
    srcs = ["dump_bazel_artifacts.cc"],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        ":bazel_artifact_reader",
        "//kythe/cxx/common:init",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)

sh_binary(
    name = "xcode_wrapper",
    srcs = ["xcode_wrapper.sh"],
    data = [
        ":cxx_extractor",
        "@bazel_tools//tools/bash/runfiles",
    ],
)

toolchain_type(name = "toolchain_type")

alias(
    name = "cxx_extractor_default",
    actual = "cxx_extractor_linux",
    visibility = [PUBLIC_VISIBILITY],
)

cxx_extractor_toolchain(
    name = "cxx_extractor_linux",
)

toolchain(
    name = "linux_toolchain",
    exec_compatible_with = [
        "@platforms//os:linux",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
    ],
    toolchain = ":cxx_extractor_linux",
    toolchain_type = ":toolchain_type",
)

cxx_extractor_toolchain(
    name = "cxx_extractor_macos",
    compiler_executable = "__KYTHE_XCODE_CLANG_PATH__",
    extractor = ":xcode_wrapper",
)

toolchain(
    name = "macos_toolchain",
    exec_compatible_with = [
        "@platforms//os:osx",
    ],
    target_compatible_with = [
        "@platforms//os:osx",
    ],
    toolchain = ":cxx_extractor_macos",
    toolchain_type = ":toolchain_type",
)

bzl_library(
    name = "toolchain_bzl",
    srcs = ["toolchain.bzl"],
)
