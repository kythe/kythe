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
    deps = [
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//third_party/guava",
        "//third_party/javac",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)
