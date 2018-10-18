load("//tools:build_rules/docker.bzl", "docker_build")

docker_build(
    name = "docker",
    image_name = "gcr.io/kythe-repo/kythe-builder",
    tags = ["manual"],
    use_cache = True,
)
