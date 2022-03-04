load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("@rules_java//java:defs.bzl", "java_binary", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

exports_files(["javac-wrapper.sh"])

java_binary(
    name = "javac_extractor",
    srcs = ["Javac9Wrapper.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.file=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
    ],
    main_class = "com.google.devtools.kythe.extractors.java.standalone.Javac9Wrapper",
    visibility = ["//visibility:public"],
    deps = [
        ":abstract_javac_wrapper",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:environment",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
    ],
)

java_library(
    name = "abstract_javac_wrapper",
    srcs = ["AbstractJavacWrapper.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:environment",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "@maven//:com_google_flogger_flogger",
    ],
)

bzl_library(
    name = "aspect_bzl",
    srcs = ["aspect.bzl"],
)
