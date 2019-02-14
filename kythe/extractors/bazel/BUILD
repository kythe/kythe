load("//tools:build_rules/docker.bzl", "docker_build")

docker_build(
    name = "docker",
    src = "Dockerfile",
    data = [
        "extract.sh",
        "extractors.BUILD",
        "extractors.WORKSPACE",
        "extractors.bazelrc",
        ":extractors",
        "//kythe/data:raw_vnames_config",
        "//kythe/data:vnames.bzl",
        "//kythe/extractors:extractors.bzl",
        "//kythe/go/platform/tools/kzip",
        "//kythe/release/base:fix_permissions.sh",
    ],
    image_name = "gcr.io/kythe-public/bazel-extractor",
    tags = ["manual"],
    use_cache = True,
)

filegroup(
    name = "extractors",
    srcs = [
        "//kythe/cxx/extractor:cxx_extractor_bazel",
        "//kythe/go/extractors/cmd/bazel:bazel_go_extractor",
        "//kythe/go/extractors/cmd/bazel:extract_kzip",
        "//kythe/java/com/google/devtools/kythe/extractors/java/bazel:java_extractor_deploy.jar",
        "//kythe/java/com/google/devtools/kythe/extractors/jvm/bazel:bazel_jvm_extractor_deploy.jar",
        "@io_kythe_lang_proto//kythe/go/extractors/proto:extract_proto_kzip",
    ],
)
