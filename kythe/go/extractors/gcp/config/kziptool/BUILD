package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/docker.bzl", "docker_build")

# This target builds a docker image which contains zipmerge.
# TODO(danielmoy): expand this to include other useful tools for managing kzips.
docker_build(
    name = "artifacts",
    src = "Dockerfile",
    image_name = "gcr.io/kythe-public/kzip-tools",
    tags = ["manual"],
    use_cache = True,
)
