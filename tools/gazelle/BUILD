load("@bazel_gazelle//:def.bzl", "gazelle_binary")

gazelle_binary(
    name = "gazelle",
    languages = [
        "@bazel_gazelle//language/proto",
        "//tools/gazelle/go",
    ],
    visibility = ["//visibility:public"],
)
