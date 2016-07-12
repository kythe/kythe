package(default_visibility = ["//kythe:default_visibility"])

load("rust_indexer_test", "rust_indexer_test")

test_suite(
    name = "testdata",
    tags = ["manual"],
    tests = [
        ":basic",
        ":function",
    ],
)

test_suite(
    name = "basic",
    tags = ["manual"],
    tests = [
        ":file_content",
        ":file_node",
        ":vardecl_local_anchor",
        ":vardecl_local_shadow",
    ],
)

test_suite(
    name = "function",
    tags = ["manual"],
    tests = [
        ":function_decl",
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

rust_indexer_test(
    name = "vardecl_local_anchor",
    srcs = ["basic/vardecl_local_anchor.rs"],
)

rust_indexer_test(
    name = "vardecl_local_shadow",
    expect_fail_verify = True,
    srcs = ["basic/vardecl_local_shadow.rs"],
)

rust_indexer_test(
    name = "function_decl",
    srcs = ["function/function_decl.rs"],
)
