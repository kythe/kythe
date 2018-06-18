package(default_visibility = ["//kythe:default_visibility"])

exports_files(glob(["*"]))

genrule(
    name = "entries",
    srcs = [
        "test.kindex",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar",
        "@bazel_tools//tools/jdk",
    ],
    outs = ["test.entries"],
    cmd = "$(JAVA) -jar $(location //kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar) $(location test.kindex) > $@",
    toolchains = ["@bazel_tools//tools/jdk:current_java_runtime"],
)
