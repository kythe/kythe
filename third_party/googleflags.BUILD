package(default_visibility = ["//visibility:public"])

load("@//tools/build_rules/cmake:cmake.bzl", "cmake_generate")

licenses(["notice"])  # BSD

filegroup(
    name = "license",
    srcs = ["COPYING.txt"],
)

cmake_generate(
    name = "headers",
    srcs =
        glob([
            "cmake/*",
            "src/*.in",
        ]) + ["CMakeLists.txt"] + [
            "src/gflags.cc",
            "src/gflags_reporting.cc",
            "src/gflags_completions.cc",
            "src/util.h",
            "src/mutex.h",
        ],
    outs = [
        "include/gflags/config.h",
        "include/gflags/gflags.h",
        "include/gflags/gflags_completions.h",
        "include/gflags/gflags_declare.h",
        "include/gflags/gflags_gflags.h",
    ],
    visibility = ["//visibility:private"],
)

cc_library(
    name = "gflags",
    srcs = [
        "include/gflags/config.h",
        "src/gflags.cc",
        "src/gflags_completions.cc",
        "src/gflags_reporting.cc",
        "src/mutex.h",
        "src/util.h",
    ],
    hdrs = [
        "include/gflags/gflags.h",
        "include/gflags/gflags_completions.h",
        "include/gflags/gflags_declare.h",
        "include/gflags/gflags_gflags.h",
    ],
    copts = [
        "-Wno-unused-local-typedef",
        "-Wno-unknown-warning-option",
        "-I$(GENDIR)/external/com_github_gflags_gflags/include/gflags",
    ],
    includes = [
        "include",
    ],
)
