package(default_visibility = ["//kythe:default_visibility"])

load("//tools/build_rules/docker", "docker_build")

java_binary(
    name = "javac_extractor",
    srcs = [
        "AbstractJavacWrapper.java",
        "Javac8Wrapper.java",
    ],
    main_class = "com.google.devtools.kythe.extractors.java.standalone.Javac8Wrapper",
    deps = [
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/guava",
        "//third_party/openjdk/lib:tools_jar",
        "//third_party/proto:protobuf_java",
    ],
)

docker_build(
    name = "docker",
    src = "Dockerfile",
    data = [
        "javac-wrapper.sh",
        ":javac_extractor_deploy.jar",
        "//kythe/release/base:fix_permissions.sh",
    ],
    image_name = "gcr.io/kythe_repo/kythe-javac-extractor",
    use_cache = True,
)
