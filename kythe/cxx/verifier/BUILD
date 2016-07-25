package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/lexyacc.bzl", "genlex", "genyacc")

genyacc(
    name = "parser",
    src = "assertions.yy",
)

genlex(
    name = "lexer",
    srcs = [
        "assertions.lex",
        ":parser",
    ],
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
        "location.hh",
        "pretty_printer.h",
        "stack.hh",
        ":parser",
    ],
    copts = [
        "-Wno-non-virtual-dtor",
        "-Wno-deprecated-register",
        "-Wno-implicit-fallthrough",
    ],
    deps = [
        "//external:glog",
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
        "//external:gflags",
        "//external:glog",
        "//external:proto/protobuf",
        "//kythe/proto:storage_proto_cc",
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
        "//external:gflags",
        "//external:glog",
        "//external:proto/protobuf",
        "//kythe/proto:storage_proto_cc",
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
        "//external:glog",
        "//external:googletest",
        "//external:proto/protobuf",
        "//kythe/proto:storage_proto_cc",
    ],
)

cc_test(
    name = "test",
    size = "small",
    deps = [
        ":testlib",
    ],
)
