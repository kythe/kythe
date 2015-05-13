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
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)

java_library(
    name = "file_vnames",
    srcs = [
        "FileVNames.java",
    ],
    deps = [
        "//kythe/proto:storage_proto_java",
        "//third_party/gson",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
        "//third_party/re2j",
    ],
)
