load("//tools:build_rules/docker.bzl", "docker_build")

docker_build(
    name = "docker",
    image_name = "gcr.io/kythe_repo/kythe-builder",
    tags = ["manual"],
    use_cache = True,
)
