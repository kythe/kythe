package(
    default_visibility = ["//visibility:public"],
)

config_setting(
    name = "darwin",
    values = {"host_cpu": "darwin"},
)

config_setting(
    name = "k8",
    values = {"host_cpu": "k8"},
)

exports_files([
    "cargo.bzl",
    "run_cargo.sh",
])
