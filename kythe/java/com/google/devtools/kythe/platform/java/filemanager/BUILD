package(default_visibility = ["//visibility:public"])

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
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_proto",
        "//third_party:protobuf",
        "//third_party/guava",
    ],
)
