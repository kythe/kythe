package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # The Universal Permissive License (UPL), Version 1.0

load("@io_kythe//tools/build_rules/lexyacc:lexyacc.bzl", "genlex", "genyacc")

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

genyacc(
    name = "parser",
    src = "src/parser/parser.yy",
    extra_outs = [
        "src/parser/stack.hh",
    ],
    header_out = "src/parser/parser.hh",
    source_out = "src/parser/parser.cc",
)

genlex(
    name = "scanner",
    src = "src/parser/scanner.ll",
    out = "scanner.yy.cc",
    includes = [":parser"],
)

cc_library(
    name = "souffle",
    srcs = glob(
        ["src/**/*.cpp"],
        exclude = [
            "src/include/**",
            "src/tests/**",
            "src/ram/tests/**",
            "src/ast/tests/**",
            "src/interpreter/tests/**",
            "src/main.cpp",
            "src/souffle_prof.cpp",
        ],
    ) + [
        ":scanner",
        ":parser",
    ],
    hdrs = glob(["src/**/*.h"]) + [":parser"],
    includes = [
        "src",
        "src/include",
    ],
    deps = [
        "@org_sourceware_libffi//:libffi",
    ],
)
