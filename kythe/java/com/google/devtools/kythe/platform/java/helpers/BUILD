package(default_visibility = ["//visibility:public"])

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
        "//kythe/java/com/google/devtools/kythe/util",
        "//third_party/guava",
        "//third_party/jsr305_annotations:jsr305",
        "//third_party/openjdk/lib:tools_jar",
        "//third_party/proto:protobuf_java",
    ],
)
