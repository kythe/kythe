load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("@aspect_bazel_lib//lib:expand_template.bzl", "expand_template_rule")

package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # Authored by Google, Apache 2.0

exports_files(glob(["*"]))

filegroup(
    name = "grep-includes",
    srcs = ["dummy.sh"],
)

bzl_library(
    name = "toolchain_utils",
    srcs = ["toolchain_utils.bzl"],
)

filegroup(
    name = "clang_tidy_deps",
    srcs = [
        "@llvm-project//clang:builtin_headers_gen",
        "@llvm-project//clang-tools-extra/clang-tidy",
    ],
)

expand_template_rule(
    name = "clang_tidy_gen",
    out = "run_clang_tidy.sh",
    data = [
        "@llvm-project//clang:builtin_headers_gen",
        "@llvm-project//clang-tools-extra/clang-tidy",
    ],
    is_executable = True,
    substitutions = {
        "@RESOURCES@": "$(execpaths @llvm-project//clang:builtin_headers_gen)",
        "@CLANG_TIDY@": "$(execpath @llvm-project//clang-tools-extra/clang-tidy)",
    },
    template = "run_clang_tidy.sh.in",
)

sh_binary(
    name = "clang_tidy",
    srcs = ["run_clang_tidy.sh"],
    data = [":clang_tidy_deps"],
)
