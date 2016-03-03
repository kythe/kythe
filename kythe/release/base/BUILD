package(default_visibility = [
    "//kythe/extractors:__subpackages__",
    "//kythe/release:__subpackages__",
])

load("//tools:build_rules/docker.bzl", "docker_build")

exports_files(
    ["fix_permissions.sh"],
    visibility = ["//visibility:public"],
)

docker_build(
    name = "base",
    data = ["fix_permissions.sh"],
    image_name = "google/kythe-base",
    tags = ["manual"],
    use_cache = True,
)
