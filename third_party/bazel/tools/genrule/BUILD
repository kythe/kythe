licenses(["unencumbered"])  # Authored by Google, Apache 2.0

exports_files(["genrule-setup.sh"])

filegroup(
    name = "srcs",
    srcs = [
        "BUILD",
        "genrule-setup.sh",
    ],
    visibility = [
        "//third_party/bazel/tools:__pkg__",
        "//tools:__pkg__",
    ],
)
