load("//tools/build_rules/docker", "docker_build")

docker_build(
    name = "release",
    src = "Dockerfile",
    data = [
        "kythe.sh",
        "maven_extractor.sh",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/go/platform/tools:dedup_stream",
        "//kythe/go/storage/tools:directory_indexer",
        "//kythe/go/storage/tools:write_entries",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar",
        "//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar",
    ],
    image_name = "google/kythe",
    use_cache = True,
    deps = [
        "//kythe/release/base",
    ],
)
