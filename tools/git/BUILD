load("@npm//:@commitlint/cli/package_json.bzl", "bin")

bin.commitlint_binary(
    name = "commitlint",
    data = [
        "//:node_modules/@commitlint/config-conventional",
    ],
    tags = ["manual"],
    visibility = ["//visibility:public"],
)
