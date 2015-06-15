load("/tools/build_rules/docker", "docker_build")

docker_build(
    name = "docker",
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
    tags = ["manual"],
    use_cache = True,
    deps = [
        "//kythe/release/base",
    ],
)

release_version = "v0.0.2"

genrule(
    name = "release",
    srcs = [
        ":javac_extractor",
        ":javac_wrapper",
        ":cxx_extractor",
        ":java_indexer",
        ":cxx_indexer",
        ":tools",
        ":misc",
        "//kythe/web/ui",
        "//third_party:licenses",
    ],
    outs = [
        "kythe-" + release_version + ".tar.gz",
        "kythe-" + release_version + ".tar.gz.md5",
    ],
    cmd = " ".join([
        "export GENDIR=$(GENDIR);",
        "export BINDIR=$(BINDIR);",
        "$(location package_release.sh) $(location kythe-" + release_version + ".tar.gz)",
        "$(locations misc)",
        "--cp $(location java_indexer) indexers/java_indexer.jar",
        "--cp $(location cxx_indexer) indexers/cxx_indexer",
        "--cp $(location javac_extractor) extractors/javac_extractor.jar",
        "--cp $(location javac_wrapper) extractors/javac-wrapper.sh",
        "--cp $(location cxx_extractor) extractors/cxx_extractor",
        "--path tools/ $(locations tools)",
        "--relpaths kythe/web/ui/resources/public",
        "--path web/ui $(locations //kythe/web/ui)",
        "--relpaths 'third_party' --path 'third_party' $(locations //third_party:licenses)",
    ]),
    heuristic_label_expansion = False,
    tags = ["manual"],
    tools = ["package_release.sh"],
)

filegroup(
    name = "misc",
    srcs = [
        ":README.md",
        "//:LICENSE",
    ],
)

filegroup(
    name = "javac_extractor",
    srcs = ["//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar"],
)

filegroup(
    name = "javac_wrapper",
    srcs = ["//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac-wrapper.sh"],
)

filegroup(
    name = "cxx_extractor",
    srcs = ["//kythe/cxx/extractor:cxx_extractor"],
)

filegroup(
    name = "java_indexer",
    srcs = ["//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar"],
)

filegroup(
    name = "cxx_indexer",
    srcs = ["//kythe/cxx/indexer/cxx:indexer"],
)

filegroup(
    name = "tools",
    srcs = [
        "//kythe/go/platform/tools:dedup_stream",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/platform/tools:indexpack",
        "//kythe/go/platform/tools:viewindex",
        "//kythe/go/serving/tools:http_server",
        "//kythe/go/serving/tools:kwazthis",
        "//kythe/go/serving/tools:kythe",
        "//kythe/go/serving/tools:write_tables",
        "//kythe/go/storage/tools:directory_indexer",
        "//kythe/go/storage/tools:read_entries",
        "//kythe/go/storage/tools:triples",
        "//kythe/go/storage/tools:write_entries",
    ],
)
