package(default_visibility = ["//kythe:default_visibility"])

load("rust_indexer_test", "rust_indexer_test")

test_suite(
    name = "basic",
    tags = ["manual"],
    tests = [
        ":file_content",
        ":file_node",
    ],
)

rust_indexer_test(
    name = "file_node",
    srcs = ["basic/file_node.rs"],
)

rust_indexer_test(
    name = "file_content",
    # buildifier: leave-alone
    srcs = [
        "basic/file_content.rs",
        "basic/a.rs",
    ],
)
