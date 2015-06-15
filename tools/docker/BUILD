load("/tools/build_rules/docker", "docker_build")

docker_build(
    name = "docker",
    src = "Dockerfile",
    image_name = "gcr.io/kythe_repo/kythe-builder",
    tags = ["manual"],
    use_cache = True,
)
