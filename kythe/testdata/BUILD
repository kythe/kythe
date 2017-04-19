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
    # TODO(https://kythe.io/phabricator/T70)
    cmd = "CWD=\"$$PWD\"; cd /tmp && \"$$CWD\"/$(JAVA) -Xbootclasspath/p:\"$$CWD\"/$(location //third_party/javac:javac_jar) -jar \"$$CWD\"/$(location //kythe/java/com/google/devtools/kythe/analyzers/java:indexer_deploy.jar) \"$$CWD\"/$(location test.kindex) > \"$$CWD\"/$@",
)
