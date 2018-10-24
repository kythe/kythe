package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/docker.bzl", "docker_build")

# This target builds a docker image which contains java extraction artifacts
# that can be drawn upon in an a la carte fashion for composing customized
# extraction images.
docker_build(
    name = "artifacts",
    src = "Dockerfile",
    data = [
        "//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac-wrapper.sh",
        "//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar",
        "//third_party/javac:javac_jar",
    ],
    image_name = "gcr.io/kythe-public/kythe-javac-extractor-artifacts",
    tags = ["manual"],
    use_cache = True,
)
