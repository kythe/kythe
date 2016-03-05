sh_binary(
    name = "abspath",
    srcs = ["abspath.sh"],
)

genrule(
    name = "compiler_info",
    outs = ["compiler_info.txt"],
    cmd = ("echo \"BAZEL_CC=$$($(location :abspath) $(CC))\n" +
           "BAZEL_C_COMPILER=$(C_COMPILER)\" > $@"),
    tags = ["manual"],
    tools = [":abspath"],
)

# This rule generates an empty .cc file if all modules are up to date.
genrule(
    name = "check_modules",
    outs = ["modules_checked_empty.cc"],
    cmd = "$(location check.sh) && touch $@",
    tools = [
        "check.sh",
        "versions.sh",
        "//third_party/llvm:git",
    ],
    visibility = ["//visibility:public"],
)
