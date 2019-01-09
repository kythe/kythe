load("//tools:build_rules/docker.bzl", "docker_build")
load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "gotool",
    srcs = ["gotool.go"],
    deps = [
        "//kythe/go/extractors/golang",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/proto:analysis_go_proto",
        "@com_github_pborman_uuid//:go_default_library",
    ],
)

docker_build(
    name = "docker",
    src = "Dockerfile",
    data = [
        "analyze_packages.sh",
        ":gotool",
        "//kythe/go/platform/tools/kzip",
        "//kythe/release/base:fix_permissions.sh",
    ],
    image_name = "gcr.io/kythe-public/golang-extractor",
    tags = ["manual"],
)
