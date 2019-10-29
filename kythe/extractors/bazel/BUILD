load("//tools:build_rules/docker.bzl", "docker_build")

docker_build(
    name = "docker",
    src = "Dockerfile",
    data = [
        "bazel_wrapper.sh",
        "extract.sh",
        "//kythe/release",
        "//kythe/release/base:fix_permissions.sh",
        "@com_github_philwo_bazelisk//:bazelisk",
        "@maven//:javax_annotation_jsr250_api",
    ],
    image_name = "gcr.io/kythe-public/bazel-extractor",
    tags = ["manual"],
    use_cache = True,
)

filegroup(
    name = "extractors_bazelrc",
    srcs = ["extractors.bazelrc"],
    visibility = ["//kythe:default_visibility"],
)
