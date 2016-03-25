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
        "//kythe/java/com/google/devtools/kythe/util",
        "//third_party/java/jdk/langtools:javac",
        "//third_party/jsr305_annotations:jsr305",
        "//third_party/proto:protobuf_java",
    ],
)
