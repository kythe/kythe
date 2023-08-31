load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load("//tools/build_rules/lexyacc:lexyacc.bzl", "genlex", "genyacc")

package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

genyacc(
    name = "parser",
    src = "assertions.yy",
    extra_outs = [
        "location.hh",
        "stack.hh",
        "position.hh",
    ],
    header_out = "parser.yy.hh",
    source_out = "parser.yy.cc",
)

genlex(
    name = "lexer",
    src = "assertions.lex",
    out = "lexer.yy.cc",
    includes = [":parser"],
)

cc_library(
    name = "lexparse",
    srcs = [
        ":lexer",
        ":parser",
    ],
    hdrs = [
        "assertion_ast.h",
        "assertions.h",
        ":parser",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-deprecated-register",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":pretty_printer",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "souffle_interpreter",
    srcs = [
        "souffle_interpreter.cc",
    ],
    hdrs = [
        "souffle_interpreter.h",
    ],
    deps = [
        ":assertions_to_souffle",
        ":lexparse",
        "//third_party/souffle:parse_transform",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@souffle",
    ],
)

cc_test(
    name = "souffle_interpreter_test",
    srcs = [
        "souffle_interpreter_test.cc",
    ],
    deps = [
        ":lib",
        ":souffle_interpreter",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log",
    ],
)

cc_library(
    name = "assertions_to_souffle",
    srcs = [
        "assertions_to_souffle.cc",
    ],
    hdrs = [
        "assertions_to_souffle.h",
    ],
    deps = [
        ":lexparse",
        ":pretty_printer",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "assertions_to_souffle_test",
    srcs = [
        "assertions_to_souffle_test.cc",
    ],
    deps = [
        ":assertions_to_souffle",
        ":lexparse",
        ":lib",
        ":pretty_printer",
        "//third_party:gtest",
        "//third_party:gtest_main",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "souffle_entrystream_support",
    srcs = [
        "souffle_entrysteam_support.cc",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    deps = [
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
        "@souffle",
    ],
    alwayslink = True,
)

# TODO(zarko): Re-enable this target once we can build with c++17 on RBE
# (we're currently missing support for <filesystem>).
# cc_binary(
#     name = "souffle_with_kythe",
#     deps = [
#         ":souffle_entrystream_support",
#         "@souffle//:main",
#     ],
# )

cc_library(
    name = "lib",
    srcs = [
        "assertions.cc",
        "parser.yy.hh",
        "verifier.cc",
    ],
    hdrs = [
        "assertions.h",
        "verifier.h",
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
        ":lexparse",
        ":pretty_printer",
        ":souffle_interpreter",
        "//kythe/cxx/common:file_utils",
        "//kythe/cxx/common:kythe_uri",
        "//kythe/cxx/common:scope_guard",
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
        "@com_google_protobuf//:protobuf",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_binary(
    name = "verifier",
    srcs = [
        "verifier_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    features = [
        "-layering_check",  # TODO: protocolbuffers/protobuf#10889
    ],
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        ":lexparse",
        ":lib",
        "//kythe/cxx/common:init",
        "//kythe/proto:storage_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//src/google/protobuf/io",
    ],
)

cc_test(
    name = "test",
    size = "small",
    srcs = [
        "verifier_unit_test.cc",
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
        "//kythe/proto:common_cc_proto",
        "//kythe/proto:storage_cc_proto",
        "//third_party:gtest",
        "@com_google_absl//absl/log:initialize",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "pretty_printer",
    srcs = ["pretty_printer.cc"],
    hdrs = ["pretty_printer.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)
