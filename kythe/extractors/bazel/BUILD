load("//tools:build_rules/docker.bzl", "docker_build")

docker_build(
    name = "docker",
    src = "Dockerfile",
    data = [
        "bazel_wrapper.sh",
        "extract.sh",
        "//kythe/release",
        "//kythe/release/base:fix_permissions.sh",
    ],
    image_name = "gcr.io/kythe-public/bazel-extractor",
    tags = ["manual"],
    use_cache = True,
)

docker_build(
    name = "stage",
    src = "Dockerfile",
    data = [
        "bazel_wrapper.sh",
        "extract.sh",
        "//kythe/release",
        "//kythe/release/base:fix_permissions.sh",
    ],
    image_name = "gcr.io/kythe-public/bazel-extractor",
    stage_only = True,
    tags = ["manual"],
    use_cache = True,
)

filegroup(
    name = "extractors_bazelrc",
    srcs = ["extractors.bazelrc"],
    visibility = ["//kythe:default_visibility"],
)
