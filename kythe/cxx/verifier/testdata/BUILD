package(default_visibility = ["//kythe:default_visibility"])

sh_test(
    name = "basic",
    size = "small",
    srcs = [
        "test_verifier_cmd.sh",
    ],
    data = [
        "//kythe/cxx/verifier",
    ],
)

sh_test(
    name = "outputs",
    size = "small",
    srcs = [
        "test_outputs.sh",
    ],
    data = [
        "just_file_node.bin",
        "just_file_node.dot",
        "//kythe/cxx/verifier",
    ],
)
