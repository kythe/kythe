package(default_visibility = ["//kythe/release:__subpackages__"])

load("//tools/build_rules/docker", "docker_build")

exports_files(
    ["fix_permissions.sh"],
    visibility = ["//visibility:public"],
)

docker_build(
    name = "base",
    src = "Dockerfile",
    data = [
        "fix_permissions.sh",
    ],
    image_name = "google/kythe-base",
    use_cache = True,
)
