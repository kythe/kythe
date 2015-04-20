package(default_visibility = ["//kythe:default_visibility"])

load("/tools/build_rules/lexyacc", "genlex", "genyacc")

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
        ":parser",
    ],
    copts = [
        "-Wno-deprecated-register",
    ],
    includes = [
        ".",
    ],
    deps = [
        "//third_party/googlelog:glog",
    ],
)

cc_library(
    name = "lib",
    srcs = [
        "assertions.cc",
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
        "//kythe/proto:storage_proto_cc",
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/proto:protobuf",
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
        "//third_party/googleflags:gflags",
        "//third_party/googlelog:glog",
        "//third_party/proto:protobuf",
    ],
)

cc_binary(
    name = "verifier",
    deps = [
        ":cmd_lib",
    ],
)
