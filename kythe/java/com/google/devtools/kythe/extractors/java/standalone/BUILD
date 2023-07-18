load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("@rules_java//java:defs.bzl", "java_binary", "java_library")
load("//tools:build_rules/selects.bzl", select_with_or = "with_or")

package(default_visibility = ["//kythe:default_visibility"])

exports_files(["javac-wrapper.sh"])

java_binary(
    name = "javac_extractor",
    srcs = ["JavacWrapper.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.file=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
    ],
    jvm_flags = [
        "--add-exports=jdk.internal.opt/jdk.internal.opt=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.api=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.code=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.file=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
    ],
    main_class = "com.google.devtools.kythe.extractors.java.standalone.JavacWrapper",
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
    runtime_deps = [
        ":reflective_jdk_compatibility_shims",
    ] + select_with_or({
        (
            "//buildenv/java:language_version_default",
            "//buildenv/java:language_version_11",
        ): [":jdk9_compatibility_shims"],
        (
            "//buildenv/java:language_version_17",
            "//buildenv/java:language_version_19",
            "//buildenv/java:language_version_20",
            "//buildenv/java:language_version_21",
        ): [":jdk15_compatibility_shims"],
    }),
    deps = [
        ":jdk_compatibility_shims",
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

java_library(
    name = "jdk_compatibility_shims",
    srcs = ["JdkCompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
    ],
    visibility = ["//visibility:private"],
    deps = ["//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service"],
)

java_library(
    name = "jdk9_compatibility_shims",
    srcs = ["9/JdkCompatibilityShimsImpl.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
        "-Xep:PackageLocation:OFF",
    ],
    target_compatible_with = select_with_or({
        (
            "//buildenv/java:language_version_default",
            "//buildenv/java:language_version_11",
        ): [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
        "//third_party/guava",
    ],
)

java_library(
    name = "jdk15_compatibility_shims",
    srcs = ["15/JdkCompatibilityShimsImpl.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
        "-Xep:PackageLocation:OFF",
    ],
    # This is incompatible with the default test configuration and
    # bazel cquery doesn't work with objc_library targets.
    tags = ["manual"],
    target_compatible_with = select_with_or({
        (
            "//buildenv/java:language_version_17",
            "//buildenv/java:language_version_19",
            "//buildenv/java:language_version_20",
        ): [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
        "//third_party/guava",
    ],
)

java_library(
    name = "jdk21_compatibility_shims",
    srcs = ["21/JdkCompatibilityShimsImpl.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.internal.opt/jdk.internal.opt=ALL-UNNAMED",
        "-Xep:PackageLocation:OFF",
    ],
    # This is incompatible with the default test configuration and
    # bazel cquery doesn't work with objc_library targets.
    tags = ["manual"],
    target_compatible_with = select_with_or({
        (
            "//buildenv/java:language_version_21",
        ): [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
        "//third_party/guava",
    ],
)

java_library(
    name = "reflective_jdk_compatibility_shims",
    srcs = ["ReflectiveJdkCompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.main=ALL-UNNAMED",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
    ],
)

bzl_library(
    name = "aspect_bzl",
    srcs = ["aspect.bzl"],
)
