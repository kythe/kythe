load("//tools:build_rules/docker.bzl", "docker_build")

package(default_visibility = ["//kythe:default_visibility"])

# This target builds a docker image which contains the kzip tool binary.
docker_build(
    name = "artifacts",
    src = "Dockerfile",
    data = [
        "//kythe/go/platform/tools/kzip",
    ],
    image_name = "gcr.io/kythe-public/kzip-tools",
    tags = ["manual"],
    use_cache = True,
)
