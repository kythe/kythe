package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "shared",
    srcs = [
        "CompilationDescription.java",
        "CompilationFileInputComparator.java",
        "ExtractionException.java",
        "ExtractorUtils.java",
        "IndexInfoUtils.java",
    ],
    deps = [
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
    ],
)

java_library(
    name = "file_vnames",
    srcs = [
        "FileVNames.java",
    ],
    deps = [
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_code_gson_gson//jar",
        "@com_google_guava_guava//jar",
        "@com_google_re2j_re2j//jar",
    ],
)
