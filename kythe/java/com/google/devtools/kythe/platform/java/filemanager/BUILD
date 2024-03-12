load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "forwarding_standard_java_file_manager",
    srcs = ["ForwardingStandardJavaFileManager.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.api",
    ],
)

java_library(
    name = "filemanager",
    srcs = ["CompilationUnitPathFileManager.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.api",
        "jdk.compiler/com.sun.tools.javac.main",
        "jdk.compiler/com.sun.tools.javac.util",
    ],
    deps = [
        ":forwarding_standard_java_file_manager",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared/filesystem",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:java_java_proto",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_guava_guava",
        "@maven//:com_google_jimfs_jimfs",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "filetree",
    srcs = [
        "CompilationUnitFileTree.java",
    ],
    deps = [
        "//kythe/proto:analysis_java_proto",
        "@maven//:org_checkerframework_checker_qual",
    ],
)
