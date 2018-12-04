package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/docker.bzl", "docker_build")

# This target builds a docker image which contains bazel extraction artifacts
# used for prepping a repo's WORKSPACE for extraction.
docker_build(
    name = "artifacts",
    src = "Dockerfile",
    data = [
        "//kythe/examples/bazel:setup-bazel-repo.sh",
    ],
    image_name = "gcr.io/kythe-public/kythe-bazel-extractor-artifacts",
    tags = ["manual"],
    use_cache = True,
)
