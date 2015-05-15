genrule(
    name = "compiler_info",
    outs = ["compiler_info.txt"],
    cmd = "echo 'BAZEL_CC=$(CC)\nBAZEL_C_COMPILER=$(C_COMPILER)' > $@",
    tags = ["manual"],
)
