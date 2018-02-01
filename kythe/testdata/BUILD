package(default_visibility = ["//kythe:default_visibility"])

exports_files(glob(["*"]))

genrule(
    name = "entries",
    srcs = [
        "test.kindex",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar",
        "//third_party/javac:javac_jar",
        "@bazel_tools//tools/jdk",
    ],
    outs = ["test.entries"],
    cmd = "$(JAVA) -Xbootclasspath/p:$(location //third_party/javac:javac_jar) -jar $(location //kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar) $(location test.kindex) > $@",
    toolchains = ["@bazel_tools//tools/jdk:current_java_runtime"],
)
