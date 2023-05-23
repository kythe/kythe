load("//tools:build_rules/docker.bzl", "docker_build")
load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "preprocessor",
    srcs = ["preprocessor.go"],
    deps = [
        "//kythe/go/extractors/config/preprocessor/modifier",
        "//kythe/go/extractors/constants",
        "//kythe/go/util/log",
    ],
)

docker_build(
    name = "docker",
    src = "Dockerfile",
    data = [
        "//kythe/go/extractors/config/preprocessor",
    ],
    image_name = "gcr.io/kythe-public/build-preprocessor",
    tags = ["manual"],
)
