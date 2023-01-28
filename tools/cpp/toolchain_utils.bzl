"""Wrapper around @bazel_tools//tools/cpp:toolchain_utils.bzl to simplify imports."""

load("@bazel_tools//tools/cpp:toolchain_utils.bzl", _find_cpp_toolchain = "find_cpp_toolchain")

find_cpp_toolchain = _find_cpp_toolchain

def use_cpp_toolchain():
    return ["@bazel_tools//tools/cpp:toolchain_type"]
