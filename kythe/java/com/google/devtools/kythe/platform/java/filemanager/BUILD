package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "forwarding_standard_java_file_manager",
    srcs = ["ForwardingStandardJavaFileManager.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//third_party/javac",
    ],
)

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
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:java_java_proto",
        "//third_party/javac",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
    ],
)
