package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/lexyacc.bzl", "genlex", "genyacc")

genyacc(
    name = "parser",
    srcs = ["assertions.yy"],
    outs = [
        "parser.yy.cc",
        "parser.yy.hh",
        "location.hh",
        "stack.hh",
        "position.hh",
    ],
)

genlex(
    name = "lexer",
    srcs = [
        "assertions.lex",
        ":parser",
    ],
    out = "lexer.yy.cc",
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
        "pretty_printer.h",
        ":parser",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-deprecated-register",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "assertions.cc",
        "parser.yy.hh",
        "pretty_printer.cc",
        "verifier.cc",
    ],
    hdrs = [
        "assertion_ast.h",
        "assertions.h",
        "pretty_printer.h",
        "verifier.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lexparse",
        "//kythe/cxx/common:lib",
        "//kythe/proto:common_proto_cc",
        "//kythe/proto:storage_proto_cc",
        "//third_party/proto:protobuf",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "cmd_lib",
    srcs = [
        "verifier_main.cc",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/proto:storage_proto_cc",
        "//third_party/proto:protobuf",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "verifier",
    deps = [
        ":cmd_lib",
    ],
)

cc_library(
    name = "testlib",
    testonly = 1,
    srcs = [
        "verifier_unit_test.cc",
    ],
    hdrs = [
        "verifier.h",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-unused-variable",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        ":lib",
        "//kythe/proto:storage_proto_cc",
        "//third_party:gtest",
        "//third_party/proto:protobuf",
        "@com_github_google_glog//:glog",
    ],
)

cc_test(
    name = "test",
    size = "small",
    deps = [
        ":testlib",
    ],
)
