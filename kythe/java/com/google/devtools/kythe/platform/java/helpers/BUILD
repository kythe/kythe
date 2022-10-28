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
        ":jdk9_compatibility_shims",
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "jdk_compatibility_shims",
    srcs = ["JdkCompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
    ],
    visibility = ["//visibility:private"],
)

java_library(
    name = "jdk9_compatibility_shims",
    srcs = ["Jdk9CompatibilityShims.java"],
    javacopts = [
        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
        "--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":jdk_compatibility_shims",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//third_party/guava",
    ],
)

#java_library(
#    name = "jdk15_compatibility_shims",
#    srcs = ["Jdk15CompatibilityShims.java"],
#    javacopts = [
#        "--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED",
#    ],
#    visibility = ["//visibility:private"],
#    deps = [
#        ":jdk_compatibility_shims",
#        "//kythe/java/com/google/devtools/kythe/common:autoservice",
#    ],
#)
