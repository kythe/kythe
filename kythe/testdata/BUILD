package(default_visibility = ["//kythe:default_visibility"])

exports_files(glob(["*"]))

genrule(
    name = "entries",
    srcs = [
        "test.kindex",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar",
    ],
    outs = ["test.entries"],
    cmd = "$(location //kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar) $(location test.kindex) > $@",
)
