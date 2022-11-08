load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "helpers",
    srcs = [
        "BlockAnonymousSignatureGenerator.java",
        "JCTreeScanner.java",
        "JavacUtil.java",
        "MemoizedTreePathScanner.java",
        "SignatureGenerator.java",
        "SyntaxPreservingScanner.java",
    ],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.code=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.model=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED",
    ],
    deps = [
        ":jdk_compatibility_shims",
        ":reflective_jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:org_checkerframework_checker_qual",
    ] + select({
        "//buildenv/java:language_version_11": [":jdk9_compatibility_shims"],
        "//buildenv/java:language_version_17": [":jdk15_compatibility_shims"],
    }),
)

java_library(
    name = "jdk_compatibility_shims",
    srcs = ["JdkCompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
    ],
    visibility = ["//visibility:private"],
    deps = ["//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service"],
)

java_library(
    name = "jdk9_compatibility_shims",
    srcs = ["Jdk9CompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
    ],
    target_compatible_with = select({
        "//buildenv/java:language_version_11": [],
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
    srcs = ["Jdk15CompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
    ],
    # This is incompatible with the default test configuration and
    # bazel cquery doesn't work with objc_library targets.
    tags = ["manual"],
    target_compatible_with = select({
        "//buildenv/java:language_version_17": [],
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
    srcs = ["ReflectiveJdkCompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/java/com/google/devtools/kythe/util:ordered_compatibility_service",
        "//third_party/guava",
    ],
)
