load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "filesystem",
    srcs = [
        "CompilationUnitFileAttributes.java",
        "CompilationUnitFileSystem.java",
        "CompilationUnitFileSystemProvider.java",
        "CompilationUnitPath.java",
    ],
    deps = [
        ":bytechannel",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager:filetree",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
    ],
)

java_library(
    name = "bytechannel",
    srcs = ["ByteBufferByteChannel.java"],
    deps = [
        "//third_party/guava",
    ],
)
