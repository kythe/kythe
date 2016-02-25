package(default_visibility = ["//visibility:public"])

filegroup(
    name = "default-goroot",
    srcs = [
        ":bin",
        ":pkg",
        ":src",
    ],
)

filegroup(
    name = "gotool",
    srcs = ["bin/go"],
)

filegroup(
    name = "bin",
    srcs = [
        "bin/go",
    ],
)

filegroup(
    name = "src",
    srcs = glob(["src/**"]),
)

filegroup(
    name = "pkg",
    srcs = glob(["pkg/**"]),
)
