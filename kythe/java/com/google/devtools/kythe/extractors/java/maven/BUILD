load("//tools/build_rules/docker", "docker_build")

docker_build(
    name = "docker",
    src = "Dockerfile",
    image_name = "gcr.io/kythe_repo/kythe-maven-extractor",
    use_cache = True,
    deps = ["//kythe/java/com/google/devtools/kythe/extractors/java/standalone:docker"],
)
