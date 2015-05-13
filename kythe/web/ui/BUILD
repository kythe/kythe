package(default_visibility = ["//kythe:default_visibility"])

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
    outs = ["resources/public/js/main.js"],
    cmd = "\n".join([
        "lein=\"$$PWD/$(location //third_party/leiningen:lein)\"",
        "export HOME=/tmp/lein",
        "mkdir -p $$HOME",
        "cd kythe/web/ui",
        "$$lein cljsbuild once prod",
        "cd ../../..",
        "mv kythe/web/ui/resources/public/js/main.js $@",
    ]),
    heuristic_label_expansion = False,
    tags = ["manual"],
    tools = ["//third_party/leiningen:lein"],
)
