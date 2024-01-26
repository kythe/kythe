package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

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
    name = "regex",
    size = "small",
    srcs = [
        "test_verifier_regex.sh",
    ],
    data = [
        "regex_expected_error.txt",
        "regex_input.txt",
        "//kythe/cxx/verifier",
    ],
)

sh_test(
    name = "singleton",
    size = "small",
    srcs = [
        "test_singleton.sh",
    ],
    args = [
        "singleton_input.txt",
        "singleton_expected_error.txt",
    ],
    data = [
        "singleton_expected_error.txt",
        "singleton_input.txt",
        "//kythe/cxx/verifier",
    ],
)

sh_test(
    name = "singleton_inspect",
    size = "small",
    srcs = [
        "test_singleton.sh",
    ],
    args = [
        "singleton_inspect_input.txt",
        "singleton_inspect_expected_error.txt",
    ],
    data = [
        "singleton_inspect_expected_error.txt",
        "singleton_inspect_input.txt",
        "//kythe/cxx/verifier",
    ],
)

sh_test(
    name = "singleton_inspect_fs",
    size = "small",
    srcs = [
        "test_singleton_fs.sh",
    ],
    args = [
        "singleton_inspect_input_fs.txt",
        "singleton_inspect_expected_error_fs.txt",
    ],
    data = [
        "singleton_inspect_expected_error_fs.txt",
        "singleton_inspect_input_fs.txt",
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
