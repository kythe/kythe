package(default_visibility = ["//kythe:default_visibility"])

exports_files(glob(["*"]))

genrule(
    name = "entries",
    srcs = [
        "test.kzip",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar",
        "@bazel_tools//tools/jdk",
    ],
    outs = ["test.entries"],
    cmd = "$(JAVA) -jar $(location //kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar) $(location test.kzip) > $@",
    toolchains = ["@rules_java//toolchains:current_java_runtime"],
)
