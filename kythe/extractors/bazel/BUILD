load("//tools:build_rules/docker.bzl", "docker_build")

docker_build(
    name = "docker",
    src = "Dockerfile",
    data = [
        "extract.sh",
        "extractors.bazelrc",
        "//kythe/release",
        "//kythe/release/base:fix_permissions.sh",
        "@com_github_philwo_bazelisk//:bazelisk",
    ],
    image_name = "gcr.io/kythe-public/bazel-extractor",
    tags = ["manual"],
    use_cache = True,
)
