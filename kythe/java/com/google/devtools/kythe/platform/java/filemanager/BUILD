package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "filemanager",
    srcs = [
        "CompilationUnitBasedJavaFileManager.java",
        "CompilationUnitBasedJavaFileStore.java",
        "CompilationUnitFileTree.java",
        "CustomFileObject.java",
        "CustomJavaFileObject.java",
        "JavaFileStore.java",
        "JavaFileStoreBasedFileManager.java",
    ],
    deps = [
        "@bazel_tools//third_party/java/jdk/langtools:javac",
        "//external:proto/any_proto_java",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:java_proto_java",
    ],
)
