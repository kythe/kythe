package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "shared",
    srcs = [
        "CompilationDescription.java",
        "ExtractionException.java",
        "ExtractorUtils.java",
        "IndexInfoUtils.java",
    ],
    deps = [
        ":file_vnames",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "file_vnames",
    srcs = [
        "FileVNames.java",
    ],
    deps = [
        "//kythe/proto:storage_java_proto",
        "@com_google_code_gson_gson//jar",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_re2j_re2j//jar",
    ],
)
