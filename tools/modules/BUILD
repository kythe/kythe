genrule(
    name = "compiler_info",
    outs = ["compiler_info.txt"],
    cmd = "echo 'BAZEL_CC=$(CC)\nBAZEL_C_COMPILER=$(C_COMPILER)' > $@",
    tags = ["manual"],
)

# This rule generates an empty .cc file if all modules are up to date.
genrule(
    name = "check_modules",
    outs = ["modules_checked_empty.cc"],
    cmd = "$(location check.sh) && touch $@",
    tools = [
        "check.sh",
        "versions.sh",
    ],
    visibility = ["//visibility:public"],
)
