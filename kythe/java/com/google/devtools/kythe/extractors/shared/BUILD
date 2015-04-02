package(default_visibility = ["//visibility:public"])

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
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party:protobuf",
        "//third_party/guava",
    ],
)

java_library(
    name = "file_vnames",
    srcs = [
        "FileVNames.java",
    ],
    deps = [
        "//kythe/proto:storage_proto",
        "//third_party:protobuf",
        "//third_party/gson",
        "//third_party/guava",
        "//third_party/re2j",
    ],
)
