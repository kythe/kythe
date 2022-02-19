load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("@rules_java//java:defs.bzl", "java_binary", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

exports_files(["javac-wrapper.sh"])

java_binary(
    name = "javac_extractor",
    srcs = ["Javac8Wrapper.java"],
    main_class = "com.google.devtools.kythe.extractors.java.standalone.Javac8Wrapper",
    visibility = ["//visibility:public"],
    deps = [
        ":abstract_javac_wrapper",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:environment",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/javac",
    ],
)

java_binary(
    name = "javac9_extractor",
    srcs = ["Javac9Wrapper.java"],
    main_class = "com.google.devtools.kythe.extractors.java.standalone.Javac9Wrapper",
    visibility = ["//visibility:public"],
    deps = [
        ":abstract_javac_wrapper",
	":javac9_compilation_unit_from_args",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:environment",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/javac:javac9",
    ],
)

java_library(
    name = "abstract_javac_wrapper",
    srcs = ["AbstractJavacWrapper.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:environment",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "@com_google_common_flogger//api",
    ],
)

java_library(
    name = "javac9_compilation_unit_from_args",
    srcs = ["Javac9CompilationUnitFromArgs.java"],
    deps = [
        ":abstract_javac_wrapper",    
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:environment",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "@com_google_common_flogger//api",
    ],
)

bzl_library(
    name = "aspect_bzl",
    srcs = ["aspect.bzl"],
)
