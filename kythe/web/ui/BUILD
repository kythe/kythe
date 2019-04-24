package(default_visibility = ["//kythe:default_visibility"])

exports_files(["project.clj"])

# The targets in kythe/web/ui are marked "manual", so they will not build if you
# blaze build //..., due to the behavior described in
# https://docs.bazel.build/versions/master/guide.html#target-patterns.

filegroup(
    name = "ui",
    srcs = [":prod-js"] + glob(["resources/public/**"]),
    tags = ["manual"],
)

genrule(
    name = "prod-js",
    srcs = glob([
        "project.clj",
        "src-cljs/**/*.cljs",
    ]),
    outs = [
        "resources/public/js/main.js",
        "resources/public/js/licenses.txt",
    ],
    cmd = "$(location @org_leiningen//:leinbuild) $(location :project.clj) $(OUTS)",
    heuristic_label_expansion = False,
    tags = ["manual"],
    tools = ["@org_leiningen//:leinbuild"],
)
