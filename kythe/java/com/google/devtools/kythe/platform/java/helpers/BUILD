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
        "//external:guava",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/util",
        "//third_party/jsr305_annotations:jsr305",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
    ],
)
