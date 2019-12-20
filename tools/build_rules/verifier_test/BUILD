package(default_visibility = ["//visibility:public"])

load("@rules_python//python:defs.bzl", "py_binary")

exports_files(["verifier_test.sh.in"])

py_binary(
    name = "unbundle",
    srcs = ["unbundle.py"],
    python_version = "PY2",
)
