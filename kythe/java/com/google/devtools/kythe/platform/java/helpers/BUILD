load("@rules_java//java:defs.bzl", "java_library")
load("//tools:build_rules/selects.bzl", select_with_or = "with_or")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "base",
    srcs = [
        "MemoizedTreePathScanner.java",
    ],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.model",
        "jdk.compiler/com.sun.tools.javac.tree",
        "jdk.compiler/com.sun.tools.javac.util",
    ],
    deps = [
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "helpers",
    srcs = [
        "21/BlockAnonymousSignatureGenerator.java",
        "21/SignatureGenerator.java",
        "JCTreeScanner.java",
        "JavacUtil.java",
        "MemoizedTreePathScanner.java",
        "SyntaxPreservingScanner.java",
    ],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.code",
        "jdk.compiler/com.sun.tools.javac.model",
        "jdk.compiler/com.sun.tools.javac.parser",
        "jdk.compiler/com.sun.tools.javac.tree",
        "jdk.compiler/com.sun.tools.javac.util",
    ],
    javacopts = [
        "-Xep:PackageLocation:OFF",
    ],
    exports = [
        ":base",
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
        ): [":jdk15_compatibility_shims"],
        (
            "//buildenv/java:language_version_20",
            "//buildenv/java:language_version_21",
        ): [":jdk21_compatibility_shims"],
    }),
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//third_party/guava",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "jdk_compatibility_shims",
    srcs = ["JdkCompatibilityShims.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.parser",
        "jdk.compiler/com.sun.tools.javac.tree",
    ],
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
    ],
)

java_library(
    name = "jdk9_compatibility_shims",
    srcs = ["9/JdkCompatibilityShimsImpl.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.parser",
        "jdk.compiler/com.sun.tools.javac.tree",
    ],
    javacopts = [
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
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.tree",
    ],
    javacopts = [
        "-Xep:PackageLocation:OFF",
    ],
    # This is incompatible with the default test configuration and
    # bazel cquery doesn't work with objc_library targets.
    tags = ["manual"],
    target_compatible_with = select_with_or({
        (
            "//buildenv/java:language_version_17",
            "//buildenv/java:language_version_19",
        ): [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
    ],
)

java_library(
    name = "jdk20_compatibility_shims",
    srcs = ["20/JdkCompatibilityShimsImpl.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.tree",
    ],
    javacopts = [
        "-Xep:PackageLocation:OFF",
    ],
    tags = ["manual"],
    target_compatible_with = select_with_or({
        (
            "//buildenv/java:language_version_20",
        ): [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
    ],
)

java_library(
    name = "jdk21_compatibility_shims",
    srcs = ["21/JdkCompatibilityShimsImpl.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.tree",
    ],
    javacopts = [
        "-Xep:PackageLocation:OFF",
    ],
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
    ],
)

java_library(
    name = "jdk24_compatibility_shims",
    srcs = ["24/JdkCompatibilityShimsImpl.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.tree",
    ],
    javacopts = [
        "-Xep:PackageLocation:OFF",
    ],
    tags = ["manual"],
    target_compatible_with = select_with_or({
        (
            "//buildenv/java:language_version_24",
        ): [],
        "//conditions:default": ["@platforms//:incompatible"],
    }),
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
    ],
)

java_library(
    name = "reflective_jdk_compatibility_shims",
    srcs = ["ReflectiveJdkCompatibilityShimsImpl.java"],
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.parser",
        "jdk.compiler/com.sun.tools.javac.tree",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
        "//third_party/guava",
    ],
)
