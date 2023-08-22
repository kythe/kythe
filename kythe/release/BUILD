load("//tools:build_rules/docker.bzl", "docker_build")

# See setup_release.sh for instructions on creating a release

docker_build(
    name = "docker",
    data = [
        "kythe.sh",
        "maven_extractor.sh",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/go/indexer/cmd/go_indexer",
        "//kythe/go/platform/tools/dedup_stream",
        "//kythe/go/storage/tools:directory_indexer",
        "//kythe/go/storage/tools:write_entries",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar",
        "//kythe/java/com/google/devtools/kythe/analyzers/jvm:class_file_indexer_deploy.jar",
        "//kythe/java/com/google/devtools/kythe/extractors/java/standalone:javac_extractor_deploy.jar",
    ],
    image_name = "google/kythe",
    tags = ["manual"],
    use_cache = True,
    deps = ["//kythe/release/base"],
)

release_version = "v0.0.63"

genrule(
    name = "release",
    srcs = [
        "release.BUILD",
        "release.WORKSPACE",
        ":bazel_cxx_extractor",
        ":bazel_extract_kzip",
        ":bazel_go_extractor",
        ":bazel_java_extractor",
        ":bazel_jvm_extractor",
        ":bazel_proto_extractor",
        ":javac_extractor",
        ":javac_wrapper",
        ":cc_proto_metadata_plugin",
        ":cxx_extractor",
        ":go_extractor",
        ":java_indexer",
        ":jvm_indexer",
        ":cxx_indexer",
        ":go_indexer",
        ":proto_extractor",
        "@maven//:org_apache_tomcat_tomcat_annotations_api",
        ":proto_indexer",
        ":textproto_extractor",
        ":textproto_indexer",
        ":tools",
        ":misc",
        "//kythe/proto:public",
        "//third_party:licenses",
        "//kythe/extractors/bazel:extractors_bazelrc",
    ],
    outs = [
        "kythe-" + release_version + ".tar.gz",
        "kythe-" + release_version + ".tar.gz.sha256",
    ],
    cmd = " ".join([
        "export GENDIR=$(GENDIR);",
        "export BINDIR=$(BINDIR);",
        "$(location package_release.sh)",
        "$(location //kythe/go/platform/tools/shasum_tool)",
        "$(location kythe-" + release_version + ".tar.gz)",
        "$(locations misc)",
        "--cp $(location release.BUILD) BUILD",
        "--cp $(location release.WORKSPACE) WORKSPACE",
        "--cp $(location java_indexer) indexers/java_indexer.jar",
        "--cp $(location jvm_indexer) indexers/jvm_indexer.jar",
        "--cp $(location cxx_indexer) indexers/cxx_indexer",
        "--cp $(location go_indexer) indexers/go_indexer",
        "--cp $(location proto_indexer) indexers/proto_indexer",
        "--cp $(location textproto_indexer) indexers/textproto_indexer",
        "--cp $(location javac_extractor) extractors/javac_extractor.jar",
        "--cp $(location bazel_cxx_extractor) extractors/bazel_cxx_extractor",
        "--cp $(location bazel_extract_kzip) extractors/bazel_extract_kzip",
        "--cp $(location bazel_go_extractor) extractors/bazel_go_extractor",
        "--cp $(location bazel_java_extractor) extractors/bazel_java_extractor.jar",
        "--cp $(location bazel_jvm_extractor) extractors/bazel_jvm_extractor.jar",
        "--cp $(location bazel_proto_extractor) extractors/bazel_proto_extractor",
        "--cp $(location javac_wrapper) extractors/javac-wrapper.sh",
        "--cp $(location @maven//:org_apache_tomcat_tomcat_annotations_api) jsr250-api-1.0.jar",
        "--cp $(location cxx_extractor) extractors/cxx_extractor",
        "--cp $(location go_extractor) extractors/go_extractor",
        "--cp $(location proto_extractor) extractors/proto_extractor",
        "--cp $(location textproto_extractor) extractors/textproto_extractor",
        "--cp $(location cc_proto_metadata_plugin) tools/cc_proto_metadata_plugin",
        "--cp $(location //kythe/extractors/bazel:extractors_bazelrc) ./",
        "--path tools/ $(locations tools)",
        "--path proto/ $(locations //kythe/proto:public)",
        "--relpaths 'third_party' --path 'third_party' $(locations //third_party:licenses)",
    ]),
    heuristic_label_expansion = False,
    tags = ["manual"],
    tools = [
        "package_release.sh",
        "//kythe/go/platform/tools/shasum_tool",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "misc",
    srcs = [
        ":README.md",
        "//:LICENSE",
        "//:RELEASES.md",
        "//kythe/data:raw_vnames_config",
        "//kythe/data:simple_vnames_config",
        "//kythe/data:vnames.bzl",
        "//kythe/extractors:extractors.bzl",
    ],
)

sh_test(
    name = "release_test",
    timeout = "moderate",
    srcs = ["release_test.sh"],
    args = [
        # Since go binaries are not located at their bazel target path (//go/binary
        # might be in bazel-out/go/binary/host/text/binary), pass the actual
        # location of the binary directly to the shell script.
        "$(location //kythe/go/platform/tools/shasum_tool)",
    ],
    data = [
        ":release",
        "//kythe/go/platform/tools/shasum_tool",
        "//kythe/java/com/google/devtools/kythe/util:test_srcs",
        "//kythe/testdata:entries",
        "//kythe/testdata:test.kzip",
        "//third_party/guava",
        "@com_github_stedolan_jq//:jq",
    ],
    tags = [
        "local",
        "manual",
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
    name = "bazel_java_extractor",
    srcs = ["//kythe/java/com/google/devtools/kythe/extractors/java/bazel:java_extractor_deploy.jar"],
)

filegroup(
    name = "bazel_jvm_extractor",
    srcs = ["//kythe/java/com/google/devtools/kythe/extractors/jvm/bazel:bazel_jvm_extractor_deploy.jar"],
)

filegroup(
    name = "cxx_extractor",
    srcs = ["//kythe/cxx/extractor:cxx_extractor"],
)

filegroup(
    name = "bazel_cxx_extractor",
    srcs = ["//kythe/cxx/extractor:cxx_extractor_bazel"],
)

filegroup(
    name = "go_extractor",
    srcs = ["//kythe/go/extractors/cmd/gotool"],
)

filegroup(
    name = "bazel_go_extractor",
    srcs = ["//kythe/go/extractors/cmd/bazel:bazel_go_extractor"],
)

filegroup(
    name = "bazel_proto_extractor",
    srcs = ["//kythe/go/extractors/proto:extract_proto_kzip"],
)

filegroup(
    name = "bazel_extract_kzip",
    srcs = ["//kythe/go/extractors/cmd/bazel:extract_kzip"],
)

filegroup(
    name = "java_indexer",
    srcs = ["//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar"],
)

filegroup(
    name = "jvm_indexer",
    srcs = ["//kythe/java/com/google/devtools/kythe/analyzers/jvm:class_file_indexer_deploy.jar"],
)

filegroup(
    name = "cxx_indexer",
    srcs = ["//kythe/cxx/indexer/cxx:indexer"],
)

filegroup(
    name = "go_indexer",
    srcs = ["//kythe/go/indexer/cmd/go_indexer"],
)

filegroup(
    name = "proto_extractor",
    srcs = ["//kythe/cxx/extractor/proto:proto_extractor"],
)

filegroup(
    name = "proto_indexer",
    srcs = ["//kythe/cxx/indexer/proto:indexer"],
)

filegroup(
    name = "textproto_extractor",
    srcs = ["//kythe/cxx/extractor/textproto:textproto_extractor"],
)

filegroup(
    name = "textproto_indexer",
    srcs = ["//kythe/cxx/indexer/textproto:textproto_indexer"],
)

filegroup(
    name = "cc_proto_metadata_plugin",
    srcs = ["//kythe/cxx/tools:proto_metadata_plugin"],
)

filegroup(
    name = "tools",
    srcs = [
        "//kythe/cxx/verifier",
        "//kythe/go/extractors/config/runextractor",
        "//kythe/go/platform/tools/dedup_stream",
        "//kythe/go/platform/tools/entrystream",
        "//kythe/go/platform/tools/kzip",
        "//kythe/go/serving/tools:http_server",
        "//kythe/go/serving/tools:kythe",
        "//kythe/go/serving/tools:write_tables",
        "//kythe/go/storage/tools:directory_indexer",
        "//kythe/go/storage/tools:read_entries",
        "//kythe/go/storage/tools:triples",
        "//kythe/go/storage/tools:write_entries",
    ],
)
