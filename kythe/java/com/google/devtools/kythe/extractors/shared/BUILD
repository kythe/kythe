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
        "//external:guava",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
    ],
)

java_library(
    name = "file_vnames",
    srcs = [
        "FileVNames.java",
    ],
    deps = [
        "//external:gson",
        "//external:guava",
        "//external:proto/protobuf_java",
        "//external:re2j",
        "//kythe/proto:storage_proto_java",
    ],
)
