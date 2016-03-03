load("//tools:build_rules/docker.bzl", "docker_build")

docker_build(
    name = "openjdk",
    src = "Dockerfile",
    data = [
        "extract.sh",
        "//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar",
    ],
    image_name = "openjdk-extractor",
    tags = ["manual"],
    deps = ["//kythe/release/base"],
)
